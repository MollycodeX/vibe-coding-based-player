# Build miniaudio as a plain-C static library so its implementation is
# compiled exactly once and with C (not C++) rules.
add_library(miniaudio_lib STATIC miniaudio_impl.c)
target_include_directories(miniaudio_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Suppress common warnings that originate inside the miniaudio source.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(miniaudio_lib PRIVATE
        -w   # disable all warnings for this target only
    )
endif()

# ---------------------------------------------------------------------------
# ImGui SDL2 backend (platform + renderer glue code).
# The core imgui library is provided by the system package (libimgui-dev).
# The backend source files are shipped with that package under
# /usr/share/doc/libimgui-dev/examples/backends/ and are copied into
# src/imgui_backends/ for reproducible builds.
# ---------------------------------------------------------------------------
add_library(imgui_sdl2_backend STATIC
    imgui_backends/imgui_impl_sdl2.cpp
    imgui_backends/imgui_impl_sdlrenderer2.cpp
)
target_include_directories(imgui_sdl2_backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_backends
    ${IMGUI_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(imgui_sdl2_backend PUBLIC
    ${IMGUI_LIBRARIES}
    ${SDL2_LIBRARIES}
)
# Suppress warnings inside the backend glue code.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(imgui_sdl2_backend PRIVATE -w)
endif()

# ---------------------------------------------------------------------------
# Player logic library (AudioPlayer, Playlist, CLI UI, i18n, GUI).
# ---------------------------------------------------------------------------
add_library(vibe_player_lib STATIC
    AudioPlayer.cpp
    Playlist.cpp
    UI.cpp
    I18n.cpp
    GuiApp.cpp
)
target_include_directories(vibe_player_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${IMGUI_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(vibe_player_lib PUBLIC
    miniaudio_lib
    imgui_sdl2_backend
    ${IMGUI_LIBRARIES}
    ${SDL2_LIBRARIES}
)

# Platform-specific audio back-end dependencies.
if(UNIX AND NOT APPLE)
    find_library(ASOUND_LIB asound)
    if(ASOUND_LIB)
        target_link_libraries(vibe_player_lib PUBLIC ${ASOUND_LIB})
    endif()
    target_link_libraries(vibe_player_lib PUBLIC dl pthread m)
elseif(APPLE)
    target_link_libraries(vibe_player_lib PUBLIC dl pthread m
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
    )
endif()

# Main executable.
add_executable(vibe_coding_player main.cpp)
target_include_directories(vibe_coding_player PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(vibe_coding_player PRIVATE vibe_player_lib)
