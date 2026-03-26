# Build miniaudio as a plain-C static library so its implementation is
# compiled exactly once and with C (not C++) rules.
# miniaudio is now used only for the low-level audio device output (ma_device).
add_library(miniaudio_lib STATIC miniaudio_impl.c)
target_include_directories(miniaudio_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Suppress common warnings that originate inside the miniaudio source.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(miniaudio_lib PRIVATE
        -w   # disable all warnings for this target only
    )
endif()

# ---------------------------------------------------------------------------
# Required: FFmpeg libraries for audio decoding / resampling / filtering.
# Only the audio-related subset is used (no libswscale / libpostproc).
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVFORMAT  REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVCODEC   REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVUTIL    REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)
pkg_check_modules(AVFILTER  REQUIRED IMPORTED_TARGET libavfilter)

# ---------------------------------------------------------------------------
# Optional: TagLib for metadata writing
# ---------------------------------------------------------------------------
if(PkgConfig_FOUND)
    pkg_check_modules(TAGLIB IMPORTED_TARGET taglib)
endif()

# Fallback: try CMake config-mode find_package (works when TagLib ships a
# TagLibConfig.cmake but pkg-config metadata is missing or PkgConfig itself
# is unavailable).
if(NOT TAGLIB_FOUND)
    find_package(TagLib CONFIG QUIET)
    if(TagLib_FOUND)
        set(TAGLIB_FOUND TRUE)
        # TagLib's CMake config exports a TagLib::tag target.
        set(TAGLIB_CMAKE_TARGET TagLib::tag)
    endif()
endif()

# ---------------------------------------------------------------------------
# Player logic library (AudioPlayer, Playlist, CLI UI, i18n, providers).
# ---------------------------------------------------------------------------
add_library(vibe_player_lib STATIC
    AudioPlayer.cpp
    Playlist.cpp
    UI.cpp
    I18n.cpp
    MetadataProvider.cpp
    LyricsProvider.cpp
    AudioFingerprinter.cpp
    AcoustIdClient.cpp
    MetadataWriter.cpp
    CoverArtProvider.cpp
    ${PROJECT_SOURCE_DIR}/include/MetadataProvider.h
    ${PROJECT_SOURCE_DIR}/include/LyricsProvider.h
    ${PROJECT_SOURCE_DIR}/include/AudioFingerprinter.h
    ${PROJECT_SOURCE_DIR}/include/AcoustIdClient.h
    ${PROJECT_SOURCE_DIR}/include/MetadataWriter.h
    ${PROJECT_SOURCE_DIR}/include/CoverArtProvider.h
)
target_include_directories(vibe_player_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(vibe_player_lib PUBLIC
    miniaudio_lib
    PkgConfig::AVFORMAT
    PkgConfig::AVCODEC
    PkgConfig::AVUTIL
    PkgConfig::SWRESAMPLE
    PkgConfig::AVFILTER
    Qt6::Core
    Qt6::Network
)

# Enable TagLib support if found.
if(TAGLIB_FOUND)
    target_compile_definitions(vibe_player_lib PUBLIC MSCPLAYER_HAS_TAGLIB)
    if(TAGLIB_CMAKE_TARGET)
        target_link_libraries(vibe_player_lib PUBLIC ${TAGLIB_CMAKE_TARGET})
    else()
        target_link_libraries(vibe_player_lib PUBLIC PkgConfig::TAGLIB)
    endif()
    message(STATUS "TagLib found – metadata writing enabled")
else()
    message(STATUS "TagLib not found – metadata writing disabled")
endif()

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
    TranslationManager.cpp
    ${PROJECT_SOURCE_DIR}/include/PlayerController.h
    ${PROJECT_SOURCE_DIR}/include/TranslationManager.h
)

qt_add_qml_module(vibe_coding_player
    URI VibePlayer
    VERSION 1.0
    QML_FILES
        main.qml
        GlassPanel.qml
        MeshGradientBackground.qml
        ParticleField.qml
        GlowSlider.qml
        GlassButton.qml
        AlbumArtDisplay.qml
        ImmersiveLyrics.qml
    RESOURCE_PREFIX /
)

target_include_directories(vibe_coding_player PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(vibe_coding_player PRIVATE
    vibe_player_lib
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickDialogs2
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
