// I18n.cpp
// Language string tables (English and Simplified Chinese).

#include "I18n.h"

// ---------------------------------------------------------------------------
// English strings
// ---------------------------------------------------------------------------
static const I18nStrings EN = {
    /* APP_TITLE         */ "Vibe Coding Based Player",
    /* LANGUAGE_LABEL    */ "Language",
    /* LANG_EN           */ "English",
    /* LANG_ZH           */ "\xe4\xb8\xad\xe6\x96\x87", // 中文

    /* PLAY              */ "Play",
    /* PAUSE             */ "Pause",
    /* STOP              */ "Stop",
    /* NEXT              */ "Next",
    /* PREV              */ "Prev",

    /* VOLUME            */ "Volume",

    /* PLAYLIST          */ "Playlist",
    /* ADD_TRACK         */ "Add",
    /* TRACK_PATH_HINT   */ "Path to audio file...",
    /* NO_TRACKS         */ "(playlist is empty)",
    /* REMOVE            */ "Remove",

    /* STATUS_PLAYING    */ "Playing",
    /* STATUS_PAUSED     */ "Paused",
    /* STATUS_STOPPED    */ "Stopped",
    /* STATUS_NO_TRACK   */ "No track loaded",
    /* STATUS_LABEL      */ "Status",

    /* ENGINE_NOT_READY  */ "Warning: audio engine failed to initialize.",
    /* LOAD_FAILED       */ "Failed to load track.",

    /* LYRICS            */ "Lyrics",
    /* NO_LYRICS         */ "(no lyrics available)",
    /* ARTIST            */ "Artist",
    /* ALBUM             */ "Album",
};

// ---------------------------------------------------------------------------
// Simplified Chinese strings (UTF-8)
// ---------------------------------------------------------------------------
static const I18nStrings ZH = {
    /* APP_TITLE         */ "\xe9\x9f\xb3\xe4\xb9\x90\xe6\x92\xad\xe6\x94\xbe\xe5\x99\xa8",
    /* LANGUAGE_LABEL    */ "\xe8\xaf\xad\xe8\xa8\x80",           // 语言
    /* LANG_EN           */ "English",
    /* LANG_ZH           */ "\xe4\xb8\xad\xe6\x96\x87",           // 中文

    /* PLAY              */ "\xe6\x92\xad\xe6\x94\xbe",           // 播放
    /* PAUSE             */ "\xe6\x9a\x82\xe5\x81\x9c",           // 暂停
    /* STOP              */ "\xe5\x81\x9c\xe6\xad\xa2",           // 停止
    /* NEXT              */ "\xe4\xb8\x8b\xe4\xb8\x80\xe6\x9b\xb2", // 下一曲
    /* PREV              */ "\xe4\xb8\x8a\xe4\xb8\x80\xe6\x9b\xb2", // 上一曲

    /* VOLUME            */ "\xe9\x9f\xb3\xe9\x87\x8f",           // 音量

    /* PLAYLIST          */ "\xe6\x92\xad\xe6\x94\xbe\xe5\x88\x97\xe8\xa1\xa8", // 播放列表
    /* ADD_TRACK         */ "\xe6\xb7\xbb\xe5\x8a\xa0",           // 添加
    /* TRACK_PATH_HINT   */ "\xe9\x9f\xb3\xe9\xa2\x91\xe6\x96\x87\xe4\xbb\xb6\xe8\xb7\xaf\xe5\xbe\x84...", // 音频文件路径...
    /* NO_TRACKS         */ "(\xe6\x92\xad\xe6\x94\xbe\xe5\x88\x97\xe8\xa1\xa8\xe4\xb8\xba\xe7\xa9\xba)", // (播放列表为空)
    /* REMOVE            */ "\xe5\x88\xa0\xe9\x99\xa4",           // 删除

    /* STATUS_PLAYING    */ "\xe6\xad\xa3\xe5\x9c\xa8\xe6\x92\xad\xe6\x94\xbe", // 正在播放
    /* STATUS_PAUSED     */ "\xe5\xb7\xb2\xe6\x9a\x82\xe5\x81\x9c",             // 已暂停
    /* STATUS_STOPPED    */ "\xe5\xb7\xb2\xe5\x81\x9c\xe6\xad\xa2",             // 已停止
    /* STATUS_NO_TRACK   */ "\xe6\x9c\xaa\xe5\x8a\xa0\xe8\xbd\xbd\xe6\x9b\xb2\xe7\x9b\xae", // 未加载曲目
    /* STATUS_LABEL      */ "\xe7\x8a\xb6\xe6\x80\x81",           // 状态

    /* ENGINE_NOT_READY  */ "\xe8\xad\xa6\xe5\x91\x8a\xef\xbc\x9a\xe9\x9f\xb3\xe9\xa2\x91\xe5\xbc\x95\xe6\x93\x8e\xe5\x88\x9d\xe5\xa7\x8b\xe5\x8c\x96\xe5\xa4\xb1\xe8\xb4\xa5\xe3\x80\x82",
    /* LOAD_FAILED       */ "\xe5\x8a\xa0\xe8\xbd\xbd\xe6\x9b\xb2\xe7\x9b\xae\xe5\xa4\xb1\xe8\xb4\xa5\xe3\x80\x82",

    /* LYRICS            */ "\xe6\xad\x8c\xe8\xaf\x8d",           // 歌词
    /* NO_LYRICS         */ "(\xe6\x9a\x82\xe6\x97\xa0\xe6\xad\x8c\xe8\xaf\x8d)", // (暂无歌词)
    /* ARTIST            */ "\xe8\x89\xba\xe6\x9c\xaf\xe5\xae\xb6", // 艺术家
    /* ALBUM             */ "\xe4\xb8\x93\xe8\xbe\x91",           // 专辑
};

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------
const I18nStrings& i18n(Language lang) {
    return (lang == Language::ZH) ? ZH : EN;
}

Language& currentLanguage() {
    static Language lang = Language::EN;
    return lang;
}
