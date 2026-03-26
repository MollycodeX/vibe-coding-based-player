#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <string>
#include <memory>

// Forward declarations for the miniaudio device callback.
struct ma_device;
typedef unsigned int ma_uint32;

// Audio player using FFmpeg (libavformat/libavcodec/libswresample/libavfilter)
// for decoding and miniaudio (ma_device low-level API) for audio output.
// Uses the pimpl idiom so FFmpeg/miniaudio types are not exposed in this header.
class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    // Load a track file from disk.
    // Supports all formats that FFmpeg can decode (MP3, FLAC, WAV, OGG, AAC,
    // WMA, M4A, Opus, ALAC, APE, WavPack, and many more).
    // Returns true on success.
    bool loadTrack(const std::string& filePath);

    void play();
    void pause();
    void stop();

    // Volume in the range [0.0, 1.0].
    void  setVolume(float volume);
    float getVolume() const;

    // Playback position and duration in seconds.
    float getPositionSeconds() const;
    float getDurationSeconds() const;

    // Seek to a position in seconds.
    void seekTo(float seconds);

    bool               isPlaying()       const;
    const std::string& getCurrentTrack() const;
    bool               isEngineReady()   const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    // Allow the miniaudio device callback to access Impl.
    friend void maDeviceCallback(ma_device*, void*, const void*, ma_uint32);
};

#endif // AUDIOPLAYER_H
