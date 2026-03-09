#ifndef PLAYER_H
#define PLAYER_H

class PlayerControlInterface {
public:
    virtual void moveForward() = 0;
    virtual void moveBackward() = 0;
    virtual void turnLeft() = 0;
    virtual void turnRight() = 0;
    virtual ~PlayerControlInterface() {}
};

#endif // PLAYER_H