// GuiApp.h
// Dear ImGui + SDL2 graphical front-end for the vibe player.

#ifndef GUIAPP_H
#define GUIAPP_H

#include "AudioPlayer.h"
#include "Playlist.h"
#include <string>

// Forward-declare SDL types to avoid pulling SDL2 headers into every TU.
struct SDL_Window;
struct SDL_Renderer;

class GuiApp {
public:
    GuiApp(AudioPlayer& player, Playlist& playlist);
    ~GuiApp();

    // Initialise SDL2, create a window and ImGui context.
    // Returns true on success.
    bool init();

    // Run the main event/render loop until the user closes the window.
    void run();

    // Tear down the ImGui context, renderer and SDL window.
    void shutdown();

private:
    void loadCjkFont();
    void buildUI();

    AudioPlayer& player_;
    Playlist&    playlist_;

    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    // Add-track text input buffer
    char trackInputBuf_[1024] = {};
};

#endif // GUIAPP_H
