// I18n.h
// Simple compile-time multi-language string table.
// Supports English (EN) and Simplified Chinese (ZH).

#ifndef I18N_H
#define I18N_H

// All UI-visible strings stored in one plain struct so every language
// is a single static constant – no allocations, no lookups at runtime.
struct I18nStrings {
    // Window / app
    const char* APP_TITLE;
    const char* LANGUAGE_LABEL;
    const char* LANG_EN;
    const char* LANG_ZH;

    // Player controls
    const char* PLAY;
    const char* PAUSE;
    const char* STOP;
    const char* NEXT;
    const char* PREV;

    // Volume
    const char* VOLUME;

    // Playlist panel
    const char* PLAYLIST;
    const char* ADD_TRACK;
    const char* TRACK_PATH_HINT;
    const char* NO_TRACKS;
    const char* REMOVE;

    // Status line
    const char* STATUS_PLAYING;
    const char* STATUS_PAUSED;
    const char* STATUS_STOPPED;
    const char* STATUS_NO_TRACK;
    const char* STATUS_LABEL;

    // Errors / notices
    const char* ENGINE_NOT_READY;
    const char* LOAD_FAILED;
};

enum class Language { EN, ZH };

// Returns the string table for the requested language.
const I18nStrings& i18n(Language lang);

// Global current language (defaults to EN).
Language& currentLanguage();

// Convenience: returns strings for the current language.
inline const I18nStrings& tr() { return i18n(currentLanguage()); }

#endif // I18N_H
