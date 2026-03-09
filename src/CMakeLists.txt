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

# Player logic library (AudioPlayer, Playlist, UI).
add_library(vibe_player_lib STATIC
    AudioPlayer.cpp
    Playlist.cpp
    UI.cpp
)
target_include_directories(vibe_player_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(vibe_player_lib PUBLIC miniaudio_lib)

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
