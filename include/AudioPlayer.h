#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <string>
#include <memory>

// Concrete audio player backed by miniaudio (ma_engine high-level API).
// Uses the pimpl idiom so miniaudio types are not exposed in this header.
class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    // Load a track file from disk (WAV / MP3 / FLAC / OGG supported).
    // Returns true on success.
    bool loadTrack(const std::string& filePath);

    void play();
    void pause();
    void stop();

    // Volume in the range [0.0, 1.0].
    void  setVolume(float volume);
    float getVolume() const;

    bool               isPlaying()       const;
    const std::string& getCurrentTrack() const;
    bool               isEngineReady()   const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // AUDIOPLAYER_H
