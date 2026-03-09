// AudioPlayer.cpp
// Real audio playback implementation using the miniaudio high-level engine API.

#include "AudioPlayer.h"
#include "miniaudio.h"
#include <iostream>

// Suppress verbose ALSA internal error messages on Linux.
#if defined(__linux__)
#include <alsa/asoundlib.h>
namespace {
void silentAlsaErrorHandler(const char*, int, const char*, int, const char*, ...) {}
} // namespace
#endif

// ---------------------------------------------------------------------------
// Internal implementation details (pimpl)
// ---------------------------------------------------------------------------
struct AudioPlayer::Impl {
    ma_engine engine;
    ma_sound  sound;
    bool      engineInitialized = false;
    bool      soundLoaded       = false;
    std::string currentTrack;
    float     volume            = 1.0f;
};

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
AudioPlayer::AudioPlayer() : pImpl(std::make_unique<Impl>()) {
#if defined(__linux__)
    snd_lib_error_set_handler(silentAlsaErrorHandler);
#endif
    ma_result result = ma_engine_init(NULL, &pImpl->engine);
    if (result != MA_SUCCESS) {
        std::cerr << "[AudioPlayer] Failed to initialize audio engine (error "
                  << result << "). Playback will be unavailable.\n";
    } else {
        pImpl->engineInitialized = true;
    }
}

AudioPlayer::~AudioPlayer() {
    stop();
    if (pImpl->soundLoaded) {
        ma_sound_uninit(&pImpl->sound);
    }
    if (pImpl->engineInitialized) {
        ma_engine_uninit(&pImpl->engine);
    }
}

// ---------------------------------------------------------------------------
// Playback control
// ---------------------------------------------------------------------------
bool AudioPlayer::loadTrack(const std::string& filePath) {
    if (!pImpl->engineInitialized) {
        std::cerr << "[AudioPlayer] Audio engine not ready.\n";
        return false;
    }

    // Stop and release the previously loaded sound first.
    stop();
    if (pImpl->soundLoaded) {
        ma_sound_uninit(&pImpl->sound);
        pImpl->soundLoaded = false;
        pImpl->currentTrack.clear();
    }

    ma_result result = ma_sound_init_from_file(
        &pImpl->engine,
        filePath.c_str(),
        MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
        NULL, NULL,
        &pImpl->sound);

    if (result != MA_SUCCESS) {
        std::cerr << "[AudioPlayer] Failed to load \"" << filePath
                  << "\" (error " << result << ").\n";
        return false;
    }

    pImpl->soundLoaded  = true;
    pImpl->currentTrack = filePath;
    ma_sound_set_volume(&pImpl->sound, pImpl->volume);
    return true;
}

void AudioPlayer::play() {
    if (!pImpl->soundLoaded) {
        std::cerr << "[AudioPlayer] No track loaded.\n";
        return;
    }
    ma_sound_start(&pImpl->sound);
}

void AudioPlayer::pause() {
    if (!pImpl->soundLoaded) return;
    // ma_sound_stop pauses without resetting the playback cursor.
    ma_sound_stop(&pImpl->sound);
}

void AudioPlayer::stop() {
    if (!pImpl->soundLoaded) return;
    ma_sound_stop(&pImpl->sound);
    ma_sound_seek_to_pcm_frame(&pImpl->sound, 0);
}

// ---------------------------------------------------------------------------
// Volume
// ---------------------------------------------------------------------------
void AudioPlayer::setVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    pImpl->volume = volume;
    if (pImpl->soundLoaded) {
        ma_sound_set_volume(&pImpl->sound, pImpl->volume);
    }
    if (pImpl->engineInitialized) {
        ma_engine_set_volume(&pImpl->engine, pImpl->volume);
    }
}

float AudioPlayer::getVolume() const {
    return pImpl->volume;
}

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------
bool AudioPlayer::isPlaying() const {
    if (!pImpl->soundLoaded) return false;
    return ma_sound_is_playing(&pImpl->sound) == MA_TRUE;
}

const std::string& AudioPlayer::getCurrentTrack() const {
    return pImpl->currentTrack;
}

bool AudioPlayer::isEngineReady() const {
    return pImpl->engineInitialized;
}
