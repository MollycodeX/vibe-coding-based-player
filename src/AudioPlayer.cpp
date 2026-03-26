// AudioPlayer.cpp
// Audio playback using FFmpeg for decoding and miniaudio low-level device API
// for output.  FFmpeg libraries used: libavformat, libavcodec, libavutil,
// libswresample, libavfilter.

#include "AudioPlayer.h"
#include "miniaudio.h"

#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}

#if defined(__linux__)
#include <alsa/asoundlib.h>
namespace {
void silentAlsaErrorHandler(const char*, int, const char*, int, const char*, ...) {}
} // namespace
#endif

// ---------------------------------------------------------------------------
// Ring buffer – lock-free single-producer / single-consumer byte buffer.
// ---------------------------------------------------------------------------
namespace {

class RingBuffer {
public:
    explicit RingBuffer(size_t capacity)
        : buf_(capacity), capacity_(capacity), readPos_(0), writePos_(0) {}

    // Returns the number of readable bytes.
    size_t available() const {
        size_t w = writePos_.load(std::memory_order_acquire);
        size_t r = readPos_.load(std::memory_order_relaxed);
        return w - r;
    }

    size_t freeSpace() const { return capacity_ - available(); }

    // Write at most `len` bytes.  Returns the number actually written.
    size_t write(const uint8_t* data, size_t len) {
        size_t free = freeSpace();
        if (len > free)
            len = free;
        size_t w = writePos_.load(std::memory_order_relaxed);
        for (size_t i = 0; i < len; ++i)
            buf_[(w + i) % capacity_] = data[i];
        writePos_.store(w + len, std::memory_order_release);
        return len;
    }

    // Read at most `len` bytes.  Returns the number actually read.
    size_t read(uint8_t* out, size_t len) {
        size_t avail = available();
        if (len > avail)
            len = avail;
        size_t r = readPos_.load(std::memory_order_relaxed);
        for (size_t i = 0; i < len; ++i)
            out[i] = buf_[(r + i) % capacity_];
        readPos_.store(r + len, std::memory_order_release);
        return len;
    }

    void clear() {
        readPos_.store(0, std::memory_order_relaxed);
        writePos_.store(0, std::memory_order_relaxed);
    }

private:
    std::vector<uint8_t> buf_;
    size_t capacity_;
    std::atomic<size_t> readPos_;
    std::atomic<size_t> writePos_;
};

} // namespace

// ---------------------------------------------------------------------------
// Constants for the output format (what miniaudio device receives).
// ---------------------------------------------------------------------------
static constexpr int    kOutputSampleRate = 44100;
static constexpr int    kOutputChannels   = 2;
// We output 16-bit signed PCM (ma_format_s16).
static constexpr size_t kBytesPerSample   = 2;                          // int16
static constexpr size_t kFrameSize        = kOutputChannels * kBytesPerSample; // 4
// Ring buffer size: ~2 seconds of audio.
static constexpr size_t kRingBufferSize   = kOutputSampleRate * kFrameSize * 2;
// Volume threshold below which we apply scaling (avoids unnecessary work at
// full volume).
static constexpr float  kVolumeEpsilon    = 1e-6f;
// Minimum number of free frames before we bother decoding more data.
// ~256 frames ≈ 5.8 ms at 44.1 kHz – avoids decoding tiny chunks.
static constexpr size_t kMinFillFrames    = 256;

// ---------------------------------------------------------------------------
// Pimpl implementation
// ---------------------------------------------------------------------------
struct AudioPlayer::Impl {
    // -- miniaudio device (output only) --
    ma_device device{};
    bool      deviceInitialized = false;

    // -- FFmpeg demux / decode / resample state --
    AVFormatContext* fmtCtx      = nullptr;
    AVCodecContext*  codecCtx    = nullptr;
    SwrContext*      swrCtx      = nullptr;
    int              audioStream = -1;

    // -- libavfilter graph (pass-through, extensible) --
    AVFilterGraph*   filterGraph = nullptr;
    AVFilterContext* bufferSrc   = nullptr;
    AVFilterContext* bufferSink  = nullptr;

    // -- shared state --
    RingBuffer       ring{kRingBufferSize};
    std::mutex       decodeMtx;            // protects FFmpeg contexts during seek
    std::string      currentTrack;
    float            volume       = 1.0f;
    std::atomic<bool> playing{false};
    std::atomic<bool> trackLoaded{false};
    std::atomic<float> positionSec{0.0f};
    float             durationSec = 0.0f;

    // -- helpers --
    void closeTrack();
    bool initFilterGraph();
    void decodeAndFillRing();
};

// ---------------------------------------------------------------------------
// miniaudio device data callback – pulls PCM from the ring buffer.
// ---------------------------------------------------------------------------
void maDeviceCallback(ma_device* dev, void* pOutput, const void* /*pInput*/,
                             ma_uint32 frameCount) {
    auto* impl = static_cast<AudioPlayer::Impl*>(dev->pUserData);
    auto* out  = static_cast<uint8_t*>(pOutput);
    size_t needed = static_cast<size_t>(frameCount) * kFrameSize;

    if (!impl->playing.load(std::memory_order_relaxed)) {
        std::memset(out, 0, needed);
        return;
    }

    // Try to decode more data into the ring buffer.
    impl->decodeAndFillRing();

    size_t got = impl->ring.read(out, needed);

    // Apply volume scaling on the s16 samples.
    float vol = impl->volume;
    if (vol < 1.0f - kVolumeEpsilon) {
        auto* samples = reinterpret_cast<int16_t*>(out);
        size_t count  = got / sizeof(int16_t);
        for (size_t i = 0; i < count; ++i)
            samples[i] = static_cast<int16_t>(static_cast<float>(samples[i]) * vol);
    }

    // Silence any remainder if ring didn't have enough.
    if (got < needed)
        std::memset(out + got, 0, needed - got);
}

// ---------------------------------------------------------------------------
// Close the currently loaded FFmpeg track (demux / decoder / resample / filter).
// ---------------------------------------------------------------------------
void AudioPlayer::Impl::closeTrack() {
    trackLoaded.store(false, std::memory_order_relaxed);
    playing.store(false, std::memory_order_relaxed);

    if (filterGraph) {
        avfilter_graph_free(&filterGraph);
        filterGraph = nullptr;
        bufferSrc   = nullptr;
        bufferSink  = nullptr;
    }
    if (swrCtx) {
        swr_free(&swrCtx);
    }
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
    }
    if (fmtCtx) {
        avformat_close_input(&fmtCtx);
    }
    ring.clear();
    positionSec.store(0.0f, std::memory_order_relaxed);
    durationSec = 0.0f;
    currentTrack.clear();
}

// ---------------------------------------------------------------------------
// Initialise a simple pass-through libavfilter graph.
// Source (abuffer) → Sink (abuffersink).
// This graph can later be extended with EQ / DSP filters.
// ---------------------------------------------------------------------------
bool AudioPlayer::Impl::initFilterGraph() {
    filterGraph = avfilter_graph_alloc();
    if (!filterGraph)
        return false;

    // ---- source filter ----
    const AVFilter* abuffer = avfilter_get_by_name("abuffer");
    if (!abuffer)
        return false;

    // Build an argument string that describes the decoder output format.
    char args[256];
    // Use the codec's existing channel layout to preserve the original
    // configuration (e.g. 5.1, 7.1, etc.) rather than reconstructing from
    // channel count alone.
    char layoutDesc[64] = {};
    av_channel_layout_describe(&codecCtx->ch_layout, layoutDesc, sizeof(layoutDesc));
    std::snprintf(args, sizeof(args),
                  "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
                  codecCtx->time_base.num ? codecCtx->time_base.num : 1,
                  codecCtx->time_base.den ? codecCtx->time_base.den : codecCtx->sample_rate,
                  codecCtx->sample_rate,
                  av_get_sample_fmt_name(codecCtx->sample_fmt),
                  layoutDesc);

    int ret = avfilter_graph_create_filter(&bufferSrc, abuffer, "in", args, nullptr,
                                           filterGraph);
    if (ret < 0)
        return false;

    // ---- sink filter ----
    const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
    if (!abuffersink)
        return false;

    ret = avfilter_graph_create_filter(&bufferSink, abuffersink, "out", nullptr,
                                       nullptr, filterGraph);
    if (ret < 0)
        return false;

    // Configure sink to request the output format we want.
    static const enum AVSampleFormat outFmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
    ret = av_opt_set_int_list(bufferSink, "sample_fmts", outFmts, AV_SAMPLE_FMT_NONE,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
        return false;

    static const int outRates[] = {kOutputSampleRate, -1};
    ret = av_opt_set_int_list(bufferSink, "sample_rates", outRates, -1,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
        return false;

    AVChannelLayout outChLayout{};
    av_channel_layout_default(&outChLayout, kOutputChannels);
    ret = av_opt_set(bufferSink, "ch_layouts", "stereo", AV_OPT_SEARCH_CHILDREN);
    av_channel_layout_uninit(&outChLayout);
    if (ret < 0)
        return false;

    // ---- link source → sink ----
    ret = avfilter_link(bufferSrc, 0, bufferSink, 0);
    if (ret < 0)
        return false;

    ret = avfilter_graph_config(filterGraph, nullptr);
    return ret >= 0;
}

// ---------------------------------------------------------------------------
// Decode packets from FFmpeg and push resampled PCM into the ring buffer.
// Called from the miniaudio device callback (audio thread).
// ---------------------------------------------------------------------------
void AudioPlayer::Impl::decodeAndFillRing() {
    std::lock_guard<std::mutex> lock(decodeMtx);
    if (!fmtCtx || !codecCtx)
        return;

    // Fill the ring buffer up to ~75 % capacity to keep latency low.
    const size_t targetFill = ring.freeSpace();
    if (targetFill < kFrameSize * kMinFillFrames)
        return; // already full enough

    AVPacket*  pkt   = av_packet_alloc();
    AVFrame*   frame = av_frame_alloc();
    AVFrame*   filtFrame = av_frame_alloc();
    if (!pkt || !frame || !filtFrame) {
        av_packet_free(&pkt);
        av_frame_free(&frame);
        av_frame_free(&filtFrame);
        return;
    }

    size_t filled = 0;
    while (filled < targetFill) {
        int ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0)
            break; // EOF or error

        if (pkt->stream_index != audioStream) {
            av_packet_unref(pkt);
            continue;
        }

        ret = avcodec_send_packet(codecCtx, pkt);
        av_packet_unref(pkt);
        if (ret < 0)
            break;

        while (ret >= 0) {
            ret = avcodec_receive_frame(codecCtx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            if (ret < 0)
                break;

            // Update playback position from the decoded frame PTS.
            if (frame->pts != AV_NOPTS_VALUE) {
                AVRational tb = fmtCtx->streams[audioStream]->time_base;
                positionSec.store(
                    static_cast<float>(frame->pts) * static_cast<float>(tb.num) /
                        static_cast<float>(tb.den),
                    std::memory_order_relaxed);
            }

            // Push through the filter graph.
            if (filterGraph && bufferSrc && bufferSink) {
                int fret = av_buffersrc_add_frame(bufferSrc, frame);
                if (fret < 0) {
                    av_frame_unref(frame);
                    continue;
                }

                while (true) {
                    fret = av_buffersink_get_frame(bufferSink, filtFrame);
                    if (fret == AVERROR(EAGAIN) || fret == AVERROR_EOF)
                        break;
                    if (fret < 0)
                        break;

                    // filtFrame should already be s16/stereo/44100 from sink config.
                    size_t bytes = static_cast<size_t>(filtFrame->nb_samples) * kFrameSize;
                    filled += ring.write(filtFrame->data[0], bytes);
                    av_frame_unref(filtFrame);
                }
            } else {
                // No filter graph: resample directly with SwrContext.
                if (swrCtx) {
                    // Calculate maximum output sample count.
                    int outSamples = swr_get_out_samples(swrCtx, frame->nb_samples);
                    if (outSamples <= 0) {
                        av_frame_unref(frame);
                        continue;
                    }
                    std::vector<uint8_t> buf(static_cast<size_t>(outSamples) * kFrameSize);
                    uint8_t* outBuf = buf.data();
                    int converted = swr_convert(swrCtx, &outBuf, outSamples,
                                                const_cast<const uint8_t**>(frame->data),
                                                frame->nb_samples);
                    if (converted > 0) {
                        size_t bytes = static_cast<size_t>(converted) * kFrameSize;
                        filled += ring.write(buf.data(), bytes);
                    }
                }
            }
            av_frame_unref(frame);
        }
    }

    av_frame_free(&filtFrame);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
AudioPlayer::AudioPlayer() : pImpl(std::make_unique<Impl>()) {
#if defined(__linux__)
    snd_lib_error_set_handler(silentAlsaErrorHandler);
#endif

    // Initialise a miniaudio playback device (low-level API).
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_s16;
    config.playback.channels = kOutputChannels;
    config.sampleRate        = kOutputSampleRate;
    config.dataCallback      = maDeviceCallback;
    config.pUserData         = pImpl.get();

    ma_result res = ma_device_init(NULL, &config, &pImpl->device);
    if (res != MA_SUCCESS) {
        std::cerr << "[AudioPlayer] Failed to initialise audio device (error "
                  << res << "). Playback will be unavailable.\n";
    } else {
        pImpl->deviceInitialized = true;
        // Start the device so it's always pulling from the callback.
        ma_device_start(&pImpl->device);
    }
}

AudioPlayer::~AudioPlayer() {
    stop();
    pImpl->closeTrack();
    if (pImpl->deviceInitialized) {
        ma_device_stop(&pImpl->device);
        ma_device_uninit(&pImpl->device);
    }
}

// ---------------------------------------------------------------------------
// Load a track using FFmpeg
// ---------------------------------------------------------------------------
bool AudioPlayer::loadTrack(const std::string& filePath) {
    if (!pImpl->deviceInitialized) {
        std::cerr << "[AudioPlayer] Audio device not ready.\n";
        return false;
    }

    // Close any previously loaded track.
    stop();
    {
        std::lock_guard<std::mutex> lock(pImpl->decodeMtx);
        pImpl->closeTrack();
    }

    // ---- Open input ----
    AVFormatContext* fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, filePath.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "[AudioPlayer] Failed to open \"" << filePath << "\".\n";
        return false;
    }
    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        std::cerr << "[AudioPlayer] Failed to find stream info.\n";
        return false;
    }

    // ---- Find best audio stream ----
    int streamIdx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (streamIdx < 0) {
        avformat_close_input(&fmtCtx);
        std::cerr << "[AudioPlayer] No audio stream found.\n";
        return false;
    }

    // ---- Open decoder ----
    AVCodecParameters* par = fmtCtx->streams[streamIdx]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(par->codec_id);
    if (!codec) {
        avformat_close_input(&fmtCtx);
        std::cerr << "[AudioPlayer] Unsupported codec.\n";
        return false;
    }

    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        avformat_close_input(&fmtCtx);
        return false;
    }
    avcodec_parameters_to_context(codecCtx, par);
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        std::cerr << "[AudioPlayer] Failed to open codec.\n";
        return false;
    }

    // ---- Setup resampler (fallback, used when filter graph is absent) ----
    SwrContext* swrCtx = nullptr;
    AVChannelLayout outChLayout{};
    av_channel_layout_default(&outChLayout, kOutputChannels);

    int swrRet = swr_alloc_set_opts2(&swrCtx,
                                     &outChLayout,
                                     AV_SAMPLE_FMT_S16,
                                     kOutputSampleRate,
                                     &codecCtx->ch_layout,
                                     codecCtx->sample_fmt,
                                     codecCtx->sample_rate,
                                     0, nullptr);
    av_channel_layout_uninit(&outChLayout);
    if (swrRet < 0 || swr_init(swrCtx) < 0) {
        swr_free(&swrCtx);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        std::cerr << "[AudioPlayer] Failed to initialise resampler.\n";
        return false;
    }

    // ---- Duration ----
    float duration = 0.0f;
    if (fmtCtx->duration != AV_NOPTS_VALUE)
        duration = static_cast<float>(fmtCtx->duration) / static_cast<float>(AV_TIME_BASE);

    // ---- Commit state ----
    {
        std::lock_guard<std::mutex> lock(pImpl->decodeMtx);
        pImpl->fmtCtx      = fmtCtx;
        pImpl->codecCtx    = codecCtx;
        pImpl->swrCtx      = swrCtx;
        pImpl->audioStream = streamIdx;
        pImpl->durationSec = duration;
        pImpl->currentTrack = filePath;
        pImpl->positionSec.store(0.0f, std::memory_order_relaxed);
        pImpl->ring.clear();

        // Initialise filter graph (pass-through for now).
        if (!pImpl->initFilterGraph()) {
            // Non-fatal: fall back to SwrContext-only path.
            std::cerr << "[AudioPlayer] Filter graph init failed; using direct resample.\n";
        }
    }

    pImpl->trackLoaded.store(true, std::memory_order_release);
    return true;
}

// ---------------------------------------------------------------------------
// Playback control
// ---------------------------------------------------------------------------
void AudioPlayer::play() {
    if (!pImpl->trackLoaded.load(std::memory_order_acquire)) {
        std::cerr << "[AudioPlayer] No track loaded.\n";
        return;
    }
    pImpl->playing.store(true, std::memory_order_release);
}

void AudioPlayer::pause() {
    pImpl->playing.store(false, std::memory_order_release);
}

void AudioPlayer::stop() {
    pImpl->playing.store(false, std::memory_order_release);
    // Seek back to beginning.
    if (pImpl->trackLoaded.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(pImpl->decodeMtx);
        if (pImpl->fmtCtx && pImpl->audioStream >= 0) {
            av_seek_frame(pImpl->fmtCtx, pImpl->audioStream, 0,
                          AVSEEK_FLAG_BACKWARD);
            if (pImpl->codecCtx)
                avcodec_flush_buffers(pImpl->codecCtx);
            pImpl->ring.clear();
            pImpl->positionSec.store(0.0f, std::memory_order_relaxed);
        }
    }
}

// ---------------------------------------------------------------------------
// Volume
// ---------------------------------------------------------------------------
void AudioPlayer::setVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    pImpl->volume = volume;
}

float AudioPlayer::getVolume() const {
    return pImpl->volume;
}

// ---------------------------------------------------------------------------
// Position / duration / seek
// ---------------------------------------------------------------------------
float AudioPlayer::getPositionSeconds() const {
    return pImpl->positionSec.load(std::memory_order_relaxed);
}

float AudioPlayer::getDurationSeconds() const {
    return pImpl->durationSec;
}

void AudioPlayer::seekTo(float seconds) {
    if (!pImpl->trackLoaded.load(std::memory_order_acquire))
        return;

    std::lock_guard<std::mutex> lock(pImpl->decodeMtx);
    if (!pImpl->fmtCtx)
        return;

    int64_t ts = static_cast<int64_t>(static_cast<double>(seconds) * AV_TIME_BASE);
    av_seek_frame(pImpl->fmtCtx, -1, ts, AVSEEK_FLAG_BACKWARD);
    if (pImpl->codecCtx)
        avcodec_flush_buffers(pImpl->codecCtx);
    pImpl->ring.clear();
    pImpl->positionSec.store(seconds, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------
bool AudioPlayer::isPlaying() const {
    return pImpl->playing.load(std::memory_order_relaxed);
}

const std::string& AudioPlayer::getCurrentTrack() const {
    return pImpl->currentTrack;
}

bool AudioPlayer::isEngineReady() const {
    return pImpl->deviceInitialized;
}
