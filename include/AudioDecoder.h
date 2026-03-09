#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <cstdint>
#include <string>

// Stub audio decoder interface.
// Currently playback is handled entirely by miniaudio's built-in decoders.
// This class is reserved for future custom decoding pipelines (e.g. FFmpeg).
class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();

    // Open an audio file for decoding.  Returns true on success.
    bool initialize(const char* filePath);

    // Decode the next chunk of PCM data into outputBuffer.
    // Returns the number of bytes decoded, or -1 on error / end-of-stream.
    int decode(uint8_t** outputBuffer, int* outputBufferSize);

    // Release all internal resources.
    void cleanup();

private:
    std::string filePath_;
    bool        initialized_;
};

#endif // AUDIODECODER_H
