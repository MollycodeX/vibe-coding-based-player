// UI.cpp
// Interactive command-line interface for the audio player.

#include "UI.h"
#include <iostream>
#include <sstream>
#include <iomanip>

UI::UI(AudioPlayer& player, Playlist& playlist)
    : player_(player), playlist_(playlist) {}

void UI::printHelp() const {
    std::cout << "\n=== Vibe Player Commands ===\n"
              << "  a <file>   Add a track to the playlist\n"
              << "  l          List the playlist\n"
              << "  p          Play / Resume current track\n"
              << "  pause      Pause playback\n"
              << "  s          Stop playback\n"
              << "  n          Next track\n"
              << "  b          Previous track\n"
              << "  v <0-100>  Set volume (e.g. \"v 75\")\n"
              << "  status     Show player status\n"
              << "  h          Show this help\n"
              << "  q          Quit\n"
              << "============================\n\n";
}

void UI::printStatus() const {
    std::cout << "\n--- Status ---\n"
              << "  State   : " << (player_.isPlaying() ? "Playing" : "Stopped/Paused") << "\n";
    if (!player_.getCurrentTrack().empty()) {
        std::cout << "  Track   : " << player_.getCurrentTrack() << "\n";
    }
    std::cout << "  Volume  : "
              << std::fixed << std::setprecision(0)
              << (player_.getVolume() * 100.0f) << "%\n"
              << "  Playlist: " << playlist_.trackCount() << " track(s)";
    if (playlist_.hasTrack()) {
        std::cout << ", current index " << playlist_.currentIndex();
    }
    std::cout << "\n--------------\n\n";
}

bool UI::processCommand(const std::string& line) {
    if (line.empty()) return true;

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "q" || cmd == "quit" || cmd == "exit") {
        player_.stop();
        std::cout << "Goodbye!\n";
        return false;
    }
    else if (cmd == "h" || cmd == "help") {
        printHelp();
    }
    else if (cmd == "status") {
        printStatus();
    }
    else if (cmd == "a" || cmd == "add") {
        std::string path;
        if (std::getline(iss >> std::ws, path) && !path.empty()) {
            playlist_.addTrack(path);
            std::cout << "Added: " << path << "\n";
        } else {
            std::cout << "Usage: a <filepath>\n";
        }
    }
    else if (cmd == "l" || cmd == "list") {
        if (!playlist_.hasTrack()) {
            std::cout << "Playlist is empty. Use 'a <filepath>' to add tracks.\n";
        } else {
            std::cout << "\nPlaylist (" << playlist_.trackCount() << " track(s)):\n";
            const auto& tracks = playlist_.getTracks();
            for (int i = 0; i < static_cast<int>(tracks.size()); ++i) {
                std::cout << "  [" << i << "] "
                          << (i == playlist_.currentIndex() ? "* " : "  ")
                          << tracks[i] << "\n";
            }
            std::cout << "\n";
        }
    }
    else if (cmd == "p" || cmd == "play") {
        if (!player_.getCurrentTrack().empty()) {
            // Resume a previously loaded track.
            player_.play();
            std::cout << "Playing: " << player_.getCurrentTrack() << "\n";
        } else if (playlist_.hasTrack()) {
            if (player_.loadTrack(playlist_.currentTrack())) {
                player_.play();
                std::cout << "Playing: " << playlist_.currentTrack() << "\n";
            }
        } else {
            std::cout << "Playlist is empty. Use 'a <filepath>' to add tracks.\n";
        }
    }
    else if (cmd == "pause") {
        player_.pause();
        std::cout << "Paused.\n";
    }
    else if (cmd == "s" || cmd == "stop") {
        player_.stop();
        std::cout << "Stopped.\n";
    }
    else if (cmd == "n" || cmd == "next") {
        if (!playlist_.hasTrack()) {
            std::cout << "Playlist is empty.\n";
        } else {
            const std::string& track = playlist_.nextTrack();
            if (player_.loadTrack(track)) {
                player_.play();
                std::cout << "Next: " << track << "\n";
            }
        }
    }
    else if (cmd == "b" || cmd == "prev" || cmd == "previous") {
        if (!playlist_.hasTrack()) {
            std::cout << "Playlist is empty.\n";
        } else {
            const std::string& track = playlist_.previousTrack();
            if (player_.loadTrack(track)) {
                player_.play();
                std::cout << "Previous: " << track << "\n";
            }
        }
    }
    else if (cmd == "v" || cmd == "volume") {
        int vol = -1;
        if (iss >> vol && vol >= 0 && vol <= 100) {
            player_.setVolume(static_cast<float>(vol) / 100.0f);
            std::cout << "Volume set to " << vol << "%\n";
        } else {
            std::cout << "Usage: v <0-100>\n";
        }
    }
    else {
        std::cout << "Unknown command: \"" << cmd << "\". Type 'h' for help.\n";
    }

    return true;
}

void UI::run() {
    if (!player_.isEngineReady()) {
        std::cerr << "[Warning] Audio engine failed to initialize. "
                     "Playback will not be available.\n";
    }

    std::cout << "=== Vibe Coding Based Player ===\n"
              << "Type 'h' for help, 'q' to quit.\n\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        std::cout.flush();
        if (!std::getline(std::cin, line)) break; // EOF / Ctrl-D
        if (!processCommand(line)) break;
    }
}
