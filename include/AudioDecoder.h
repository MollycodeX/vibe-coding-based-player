#ifndef AUDIODECODER_H
#define AUDIODECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();

    bool initialize(const char* filePath);
    int decode(uint8_t** outputBuffer, int* outputBufferSize);
    void cleanup();

private:
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    int audioStreamIndex;
};

#endif // AUDIODECODER_H
