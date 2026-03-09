#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

class AudioPlayer {
public:
    virtual void play(const std::string& audioFile) = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void setVolume(int volume) = 0;
    virtual int getVolume() const = 0;
};

#endif // AUDIOPLAYER_H
