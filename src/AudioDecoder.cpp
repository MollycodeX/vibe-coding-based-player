// AudioDecoder.cpp
// Stub implementation – actual decoding is handled by miniaudio.
// This file is a placeholder for a future custom decoding pipeline.

#include "AudioDecoder.h"
#include <iostream>

AudioDecoder::AudioDecoder()
    : initialized_(false) {}

AudioDecoder::~AudioDecoder() {
    cleanup();
}

bool AudioDecoder::initialize(const char* filePath) {
    if (!filePath) return false;
    filePath_    = filePath;
    initialized_ = true;
    return true;
}

int AudioDecoder::decode(uint8_t** /*outputBuffer*/, int* /*outputBufferSize*/) {
    if (!initialized_) {
        std::cerr << "[AudioDecoder] Not initialized.\n";
        return -1;
    }
    // Stub: no custom decoding yet – miniaudio handles playback.
    return -1;
}

void AudioDecoder::cleanup() {
    filePath_.clear();
    initialized_ = false;
}