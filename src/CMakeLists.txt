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
# Player logic library (AudioPlayer, Playlist, CLI UI, i18n).
# GuiApp (Dear ImGui front-end) is superseded by the Qt Quick GUI below.
# ---------------------------------------------------------------------------
add_library(vibe_player_lib STATIC
    AudioPlayer.cpp
    AudioDecoder.cpp
    Playlist.cpp
    UI.cpp
    I18n.cpp
)
target_include_directories(vibe_player_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(vibe_player_lib PUBLIC
    miniaudio_lib
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

# ---------------------------------------------------------------------------
# Qt Quick main executable
# ---------------------------------------------------------------------------
qt_add_executable(vibe_coding_player
    main.cpp
    PlayerController.cpp
    ${PROJECT_SOURCE_DIR}/include/PlayerController.h
)

qt_add_qml_module(vibe_coding_player
    URI VibePlayer
    VERSION 1.0
    QML_FILES main.qml
    RESOURCE_PREFIX /
)

target_include_directories(vibe_coding_player PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(vibe_coding_player PRIVATE
    vibe_player_lib
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

# ---------------------------------------------------------------------------
# i18n / translations
# .ts files are the authoritative source (edit with Qt Linguist or lupdate).
# .qm files are the compiled binary translations generated from .ts via lrelease.
# To regenerate: lrelease i18n/vibe-player_zh_CN.ts -qm i18n/vibe-player_zh_CN.qm
# ---------------------------------------------------------------------------
set(QM_FILE "${CMAKE_SOURCE_DIR}/i18n/vibe-player_zh_CN.qm")

find_program(LRELEASE_EXECUTABLE
    NAMES lrelease lrelease6
    HINTS /usr/lib/qt6/bin
    DOC "Qt lrelease tool for compiling .ts to .qm"
)
if(LRELEASE_EXECUTABLE)
    add_custom_command(
        OUTPUT  "${QM_FILE}"
        COMMAND "${LRELEASE_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/i18n/vibe-player_zh_CN.ts" -qm "${QM_FILE}"
        DEPENDS "${CMAKE_SOURCE_DIR}/i18n/vibe-player_zh_CN.ts"
        COMMENT "Compiling zh_CN translation"
        VERBATIM
    )
    add_custom_target(vibe_translations DEPENDS "${QM_FILE}")
    add_dependencies(vibe_coding_player vibe_translations)
endif()

qt_add_resources(vibe_coding_player "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_SOURCE_DIR}/i18n"
    FILES "${QM_FILE}"
)
