#include "AudioPlayer.h"
#include "Playlist.h"
#include "UI.h"
#include <iostream>

int main(int argc, char* argv[]) {
    AudioPlayer player;
    Playlist    playlist;

    // Any command-line arguments are treated as audio file paths to pre-load.
    for (int i = 1; i < argc; ++i) {
        playlist.addTrack(argv[i]);
        std::cout << "Added from args: " << argv[i] << "\n";
    }

    UI ui(player, playlist);
    ui.run();

    return 0;
}
