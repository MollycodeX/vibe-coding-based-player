#ifndef PLAYER_H
#define PLAYER_H

// Abstract interface for audio player control.
class PlayerControlInterface {
public:
    virtual void play()     = 0;
    virtual void pause()    = 0;
    virtual void stop()     = 0;
    virtual void next()     = 0;
    virtual void previous() = 0;
    virtual ~PlayerControlInterface() {}
};

#endif // PLAYER_H