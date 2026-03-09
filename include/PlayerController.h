#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <vector>
#include <string>

class PlayerController {
public:
    PlayerController();
    ~PlayerController();
    void play();
    void pause();
    void next();
    void previous();
    void setVolume(int volume);

private:
    std::vector<std::string> playlist;
    int currentTrackIndex;
    int volume;
};

#endif // PLAYERCONTROLLER_H