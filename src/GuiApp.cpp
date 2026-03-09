// GuiApp.cpp
// Dear ImGui (SDL2 + SDLRenderer2 backend) graphical player front-end.
// Provides: Play/Pause/Stop/Next/Prev controls, a Volume slider,
//           a Playlist panel, and an English/Chinese language toggle.

#include "GuiApp.h"
#include "I18n.h"

#include <imgui/imgui.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <SDL2/SDL.h>

#include <cstdio>
#include <cstring>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
namespace {

// Paths to search for a CJK-capable TrueType/OpenType font.
static const char* CJK_FONT_PATHS[] = {
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/opentype/noto/NotoSerifCJK-Regular.ttc",
    "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
    "/System/Library/Fonts/PingFang.ttc",                // macOS
    "C:/Windows/Fonts/msyh.ttc",                         // Windows
    nullptr
};

bool fileExists(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

const char* findCjkFont() {
    for (int i = 0; CJK_FONT_PATHS[i]; ++i) {
        if (fileExists(CJK_FONT_PATHS[i])) return CJK_FONT_PATHS[i];
    }
    return nullptr;
}

} // namespace

// ---------------------------------------------------------------------------
// GuiApp
// ---------------------------------------------------------------------------
GuiApp::GuiApp(AudioPlayer& player, Playlist& playlist)
    : player_(player), playlist_(playlist) {}

GuiApp::~GuiApp() {
    shutdown();
}

bool GuiApp::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "[GuiApp] SDL_Init error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    window_ = SDL_CreateWindow(
        "Vibe Coding Based Player",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        860, 520,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        fprintf(stderr, "[GuiApp] SDL_CreateWindow error: %s\n", SDL_GetError());
        return false;
    }

    renderer_ = SDL_CreateRenderer(
        window_, -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        // Fallback: software renderer
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer_) {
        fprintf(stderr, "[GuiApp] SDL_CreateRenderer error: %s\n", SDL_GetError());
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    // Soften the look a bit
    ImGui::GetStyle().WindowRounding = 6.0f;
    ImGui::GetStyle().FrameRounding  = 4.0f;
    ImGui::GetStyle().GrabRounding   = 4.0f;

    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);

    loadCjkFont();

    return true;
}

void GuiApp::loadCjkFont() {
    ImGuiIO& io = ImGui::GetIO();
    const char* fontPath = findCjkFont();
    if (fontPath) {
        // Build glyph ranges: Basic Latin + CJK Unified Ideographs
        ImFontConfig cfg;
        cfg.MergeMode = false;

        // Load a combined range that covers ASCII + common CJK
        static ImVector<ImWchar> ranges;
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        builder.BuildRanges(&ranges);

        ImFont* font = io.Fonts->AddFontFromFileTTF(
            fontPath, 16.0f, &cfg, ranges.Data);
        if (font) {
            io.Fonts->Build();
            return;
        }
    }
    // Fall back to the built-in font (no CJK glyphs, but app still works)
    io.Fonts->AddFontDefault();
}

void GuiApp::run() {
    if (!player_.isEngineReady()) {
        fprintf(stderr, "%s\n", tr().ENGINE_NOT_READY);
    }

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window_))
                running = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        buildUI();

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
        SDL_RenderClear(renderer_);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer_);
    }
}

void GuiApp::shutdown() {
    if (renderer_ || window_) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
        if (window_)   { SDL_DestroyWindow(window_);    window_   = nullptr; }
        SDL_Quit();
    }
}

// ---------------------------------------------------------------------------
// UI layout
// ---------------------------------------------------------------------------
void GuiApp::buildUI() {
    const I18nStrings& s = tr();

    // Full-screen dockspace window
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGuiWindowFlags mainFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##Main", nullptr, mainFlags);

    // ---- Title bar row -------------------------------------------------------
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::Text("%s", s.APP_TITLE);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.0f);
    ImGui::Text("%s:", s.LANGUAGE_LABEL);
    ImGui::SameLine();
    if (ImGui::Button(s.LANG_EN)) currentLanguage() = Language::EN;
    ImGui::SameLine();
    if (ImGui::Button(s.LANG_ZH)) currentLanguage() = Language::ZH;

    ImGui::Separator();

    // ---- Status bar ----------------------------------------------------------
    {
        const char* stateStr = s.STATUS_NO_TRACK;
        if (!player_.getCurrentTrack().empty()) {
            stateStr = player_.isPlaying() ? s.STATUS_PLAYING : s.STATUS_STOPPED;
        }
        ImGui::Text("%s: %s", s.STATUS_LABEL, stateStr);
        if (!player_.getCurrentTrack().empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("  [%s]", player_.getCurrentTrack().c_str());
        }
    }

    ImGui::Spacing();

    // ---- Transport controls --------------------------------------------------
    {
        // Prev
        if (ImGui::Button(s.PREV, ImVec2(80, 36))) {
            if (playlist_.hasTrack()) {
                const std::string& track = playlist_.previousTrack();
                player_.loadTrack(track);
                player_.play();
            }
        }
        ImGui::SameLine();

        // Play / Pause (toggle)
        const char* playLabel = player_.isPlaying() ? s.PAUSE : s.PLAY;
        if (ImGui::Button(playLabel, ImVec2(80, 36))) {
            if (player_.isPlaying()) {
                player_.pause();
            } else {
                if (!player_.getCurrentTrack().empty()) {
                    player_.play();
                } else if (playlist_.hasTrack()) {
                    player_.loadTrack(playlist_.currentTrack());
                    player_.play();
                }
            }
        }
        ImGui::SameLine();

        // Stop
        if (ImGui::Button(s.STOP, ImVec2(80, 36))) {
            player_.stop();
        }
        ImGui::SameLine();

        // Next
        if (ImGui::Button(s.NEXT, ImVec2(80, 36))) {
            if (playlist_.hasTrack()) {
                const std::string& track = playlist_.nextTrack();
                player_.loadTrack(track);
                player_.play();
            }
        }
    }

    ImGui::Spacing();

    // ---- Volume slider -------------------------------------------------------
    {
        int vol = static_cast<int>(player_.getVolume() * 100.0f + 0.5f);
        ImGui::Text("%s", s.VOLUME);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::SliderInt("##vol", &vol, 0, 100)) {
            player_.setVolume(static_cast<float>(vol) / 100.0f);
        }
        ImGui::SameLine();
        ImGui::Text("%d%%", vol);
    }

    ImGui::Separator();

    // ---- Playlist panel ------------------------------------------------------
    ImGui::Text("%s", s.PLAYLIST);
    ImGui::Spacing();

    // Add-track row
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 90.0f);
    ImGui::InputTextWithHint("##trackInput", s.TRACK_PATH_HINT,
                             trackInputBuf_, sizeof(trackInputBuf_));
    ImGui::SameLine();
    if (ImGui::Button(s.ADD_TRACK, ImVec2(80, 0)) && trackInputBuf_[0] != '\0') {
        playlist_.addTrack(trackInputBuf_);
        trackInputBuf_[0] = '\0';
    }

    ImGui::Spacing();

    // Track list
    float listHeight = ImGui::GetContentRegionAvail().y;
    ImGui::BeginChild("##tracks", ImVec2(0, listHeight), true);
    if (!playlist_.hasTrack()) {
        ImGui::TextDisabled("%s", s.NO_TRACKS);
    } else {
        const auto& tracks = playlist_.getTracks();
        for (int i = 0; i < static_cast<int>(tracks.size()); ++i) {
            bool isCurrent = (i == playlist_.currentIndex());
            if (isCurrent) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
            }
            // Clicking a row selects and plays the track
            char label[1280];
            snprintf(label, sizeof(label), "%s##trk%d",
                     tracks[i].c_str(), i);
            if (ImGui::Selectable(label, isCurrent)) {
                // Advance playlist cursor to i
                while (playlist_.currentIndex() != i) {
                    if (i > playlist_.currentIndex())
                        playlist_.nextTrack();
                    else
                        playlist_.previousTrack();
                }
                player_.loadTrack(tracks[i]);
                player_.play();
            }
            if (isCurrent) {
                ImGui::PopStyleColor();
            }

            // Remove button on the right
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60.0f);
            char btnId[32];
            snprintf(btnId, sizeof(btnId), "%s##rm%d", s.REMOVE, i);
            if (ImGui::SmallButton(btnId)) {
                if (isCurrent) player_.stop();
                playlist_.removeTrack(i);
                break; // iterator invalidated – re-render next frame
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
