// AudioPlayer.cpp
#include <iostream>
#include <string>

class AudioPlayer {
private:
    std::string currentTrack;
    bool isPlaying;

public:
    AudioPlayer() : currentTrack(""), isPlaying(false) {}

    void loadTrack(const std::string &track) {
        currentTrack = track;
        isPlaying = false;
        std::cout << "Track loaded: " << currentTrack << std::endl;
    }

    void play() {
        if (!currentTrack.empty()) {
            isPlaying = true;
            std::cout << "Playing: " << currentTrack << std::endl;
        } else {
            std::cout << "No track loaded!" << std::endl;
        }
    }

    void pause() {
        if (isPlaying) {
            isPlaying = false;
            std::cout << "Paused: " << currentTrack << std::endl;
        } else {
            std::cout << "Track is not playing!" << std::endl;
        }
    }

    void stop() {
        isPlaying = false;
        std::cout << "Stopped: " << currentTrack << std::endl;
        currentTrack = "";
    }
};

int main() {
    AudioPlayer player;
    player.loadTrack("My Favorite Song");
    player.play();
    player.pause();
    player.stop();
    return 0;
}