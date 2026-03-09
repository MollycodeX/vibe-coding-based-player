// UI.h
// Interactive command-line interface for the audio player.

#ifndef UI_H
#define UI_H

#include "AudioPlayer.h"
#include "Playlist.h"
#include <string>

class UI {
public:
    UI(AudioPlayer& player, Playlist& playlist);

    // Enter the main interactive loop (reads stdin until 'q' or EOF).
    void run();

private:
    void printHelp()   const;
    void printStatus() const;

    // Processes a single command line.  Returns false when the loop should exit.
    bool processCommand(const std::string& line);

    AudioPlayer& player_;
    Playlist&    playlist_;
};

#endif // UI_H