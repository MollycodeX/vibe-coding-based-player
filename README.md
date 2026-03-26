# MSCPLAYER

[中文](#中文) | [English](#english)

---

## 中文

这是基于Gemini，ChatGPT和Claude使用vibe coding做出的个人测试项目

### 1. 介绍，用途及效果

计划为基于C++及其相关库开发的一款跨平台（Windows和Android，其余平台待定）音乐播放器，可进行暂停/播放，上一曲/下一曲和音量调节等基本功能，同时拥有风格化UI（类似Windows Aero），显示歌词，并基于音乐的状态、频率所显示的音乐可视化界面。

#### 技术栈

- **语言**: C++17
- **GUI框架**: Qt 6 (Qt Quick / QML)
- **音频引擎**: [FFmpeg](https://ffmpeg.org/) (libavformat/libavcodec/libswresample/libavfilter 解码) + [miniaudio](https://miniaud.io/) (跨平台音频输出)
- **音频指纹**: [Chromaprint](https://acoustid.org/chromaprint) (fpcalc命令行工具)
- **音频识别**: [AcoustID](https://acoustid.org/) 开源音频识别服务
- **在线元数据**: [MusicBrainz](https://musicbrainz.org/) 开源音乐数据库 API
- **封面获取**: [Cover Art Archive](https://coverartarchive.org/) 专辑封面获取
- **在线歌词**: [LRCLIB](https://lrclib.net/) 开源歌词数据库 API
- **元数据写入**: [TagLib](https://taglib.org/) 音频标签读写库 (可选)
- **构建系统**: CMake (≥ 3.16)
- **协议**: MIT License

#### 支持的音频格式

MP3, WAV, FLAC, OGG, AAC, WMA, M4A, Opus

### 2. 现开发阶段标准（根据开发进度实时修改）

以下功能已实现并通过测试验证：

- ✅ 基本的音频读取和操作（播放/暂停/停止/跳转/音量调节）
- ✅ 基于Qt Quick (QML)的图形用户界面，支持深色/浅色主题切换
- ✅ 播放列表管理（添加文件/文件夹、移除、选择、上一曲/下一曲循环）
- ✅ 多语言支持（英文/中文），支持运行时切换语言及系统语言自动检测
- ✅ 音频指纹识别（通过Chromaprint/fpcalc计算音频指纹，即使无元数据也能识别歌曲）
- ✅ 联网查找歌曲元数据（通过AcoustID指纹匹配 + MusicBrainz开源数据库获取艺术家、专辑、曲名）
- ✅ 联网获取专辑封面（通过Cover Art Archive获取并显示专辑封面，可嵌入音频文件）
- ✅ 联网查找歌词（通过LRCLIB开源数据库获取纯文本及同步歌词）
- ✅ 多结果选择（当查找出现多个结果时，列出GUI对话框让用户手动选择正确的元数据）
- ✅ 元数据写入（可让用户选择是否将识别到的元数据录入音频文件中，通过TagLib实现）
- ✅ UI中显示歌曲元信息（曲名、艺术家、专辑、封面）和可折叠歌词面板
- ✅ Unicode路径支持（含CJK字符的文件路径）

### 3. 构建与测试

#### 依赖

- Qt 6 (Core, Gui, Qml, Quick, QuickDialogs2, Network)
- FFmpeg 开发库 (libavformat, libavcodec, libavutil, libswresample, libavfilter)
- ALSA 开发库 (Linux)
- CMake ≥ 3.16
- C++17 编译器
- [可选] Chromaprint 工具 (`fpcalc`) — 用于音频指纹识别
- [可选] TagLib (`libtag1-dev`) — 用于将元数据写入音频文件

#### 构建 (Linux)

```bash
# 安装依赖
sudo apt-get install -y qt6-base-dev qt6-declarative-dev qt6-tools-dev-tools libasound2-dev \
    libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libavfilter-dev

# 可选：安装音频指纹和元数据写入支持
sudo apt-get install -y libchromaprint-tools libtag1-dev

# 构建
mkdir build && cd build
cmake ..
cmake --build .
```

#### 运行测试

```bash
cd build
ctest --output-on-failure
```

### 4. 项目结构

```
MSCPLAYER/
├── include/           # C++头文件
│   ├── AudioPlayer.h         # FFmpeg+miniaudio音频播放器（PIMPL模式）
│   ├── Playlist.h            # 播放列表管理
│   ├── PlayerController.h    # QML↔C++桥接控制器
│   ├── AudioFingerprinter.h  # Chromaprint音频指纹计算
│   ├── AcoustIdClient.h      # AcoustID音频识别API客户端
│   ├── MetadataProvider.h    # MusicBrainz元数据查找（多结果支持）
│   ├── CoverArtProvider.h    # Cover Art Archive专辑封面获取
│   ├── LyricsProvider.h      # LRCLIB歌词查找
│   ├── MetadataWriter.h      # TagLib元数据写入
│   ├── TranslationManager.h  # 运行时语言切换管理
│   ├── I18n.h                # 编译时多语言字符串表
│   └── ...
├── src/               # C++源代码及QML界面
│   ├── main.cpp           # Qt应用入口
│   ├── main.qml           # Qt Quick用户界面（含元数据选择和写入确认对话框）
│   └── ...
├── tests/             # 单元测试
├── i18n/              # 翻译文件 (.ts/.qm)
└── CMakeLists.txt     # 构建配置
```

### 5. 计划中的新功能

- 支持更多平台（macOS，Linux和iOS）
- 添加类似索尼DSEE的音频上采样技术和类似索尼360 upmix的音频升格技术
- 根据所播放音乐的风格智能定制其风格化音频可视化界面

---

## English

This is a personal experimental project built with vibe coding using Gemini, ChatGPT and Claude.

### 1. Introduction and Purpose

MSCPLAYER is a cross-platform music player (Windows and Android, with other platforms planned) developed with C++ and related libraries. It supports basic features such as play/pause, previous/next track, and volume control, along with a stylized UI (inspired by Windows Aero), lyrics display, and music visualization based on audio state and frequency.

#### Tech Stack

- **Language**: C++17
- **GUI Framework**: Qt 6 (Qt Quick / QML)
- **Audio Engine**: [FFmpeg](https://ffmpeg.org/) (libavformat/libavcodec/libswresample/libavfilter for decoding) + [miniaudio](https://miniaud.io/) (cross-platform audio output)
- **Audio Fingerprinting**: [Chromaprint](https://acoustid.org/chromaprint) (fpcalc CLI tool)
- **Audio Recognition**: [AcoustID](https://acoustid.org/) open-source audio recognition service
- **Online Metadata**: [MusicBrainz](https://musicbrainz.org/) open music database API
- **Cover Art**: [Cover Art Archive](https://coverartarchive.org/) album cover art retrieval
- **Online Lyrics**: [LRCLIB](https://lrclib.net/) open-source lyrics database API
- **Metadata Writing**: [TagLib](https://taglib.org/) audio tag read/write library (optional)
- **Build System**: CMake (≥ 3.16)
- **License**: MIT License

#### Supported Audio Formats

MP3, WAV, FLAC, OGG, AAC, WMA, M4A, Opus

### 2. Current Development Status (updated with progress)

The following features have been implemented and verified through testing:

- ✅ Basic audio playback operations (play/pause/stop/seek/volume control)
- ✅ Qt Quick (QML) graphical user interface with dark/light theme toggle
- ✅ Playlist management (add files/folders, remove, select, previous/next with loop)
- ✅ Multilingual support (English/Chinese) with runtime language switching and system locale auto-detection
- ✅ Audio fingerprinting (via Chromaprint/fpcalc — identifies songs even without metadata)
- ✅ Online metadata lookup (AcoustID fingerprint matching + MusicBrainz for artist, album, track name)
- ✅ Online album cover art retrieval (via Cover Art Archive, with option to embed in audio files)
- ✅ Online lyrics lookup (plain text and synced LRC lyrics via LRCLIB)
- ✅ Multiple result selection (GUI dialog for choosing the correct metadata when multiple matches are found)
- ✅ Metadata writing (optionally write recognized metadata to audio files via TagLib)
- ✅ Song metadata display in UI (title, artist, album, cover art) with collapsible lyrics panel
- ✅ Unicode path support (file paths containing CJK characters)

### 3. Build and Test

#### Dependencies

- Qt 6 (Core, Gui, Qml, Quick, QuickDialogs2, Network)
- FFmpeg development libraries (libavformat, libavcodec, libavutil, libswresample, libavfilter)
- ALSA development library (Linux)
- CMake ≥ 3.16
- C++17 compiler
- [Optional] Chromaprint tools (`fpcalc`) — for audio fingerprinting
- [Optional] TagLib (`libtag1-dev`) — for writing metadata to audio files

#### Build (Linux)

```bash
# Install dependencies
sudo apt-get install -y qt6-base-dev qt6-declarative-dev qt6-tools-dev-tools libasound2-dev \
    libavformat-dev libavcodec-dev libavutil-dev libswresample-dev libavfilter-dev

# Optional: install audio fingerprinting and metadata writing support
sudo apt-get install -y libchromaprint-tools libtag1-dev

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

#### Run Tests

```bash
cd build
ctest --output-on-failure
```

### 4. Project Structure

```
MSCPLAYER/
├── include/           # C++ header files
│   ├── AudioPlayer.h         # FFmpeg+miniaudio audio player (PIMPL pattern)
│   ├── Playlist.h            # Playlist management
│   ├── PlayerController.h    # QML ↔ C++ bridge controller
│   ├── AudioFingerprinter.h  # Chromaprint audio fingerprint calculation
│   ├── AcoustIdClient.h      # AcoustID audio recognition API client
│   ├── MetadataProvider.h    # MusicBrainz metadata lookup (multi-result)
│   ├── CoverArtProvider.h    # Cover Art Archive album art retrieval
│   ├── LyricsProvider.h      # LRCLIB lyrics lookup
│   ├── MetadataWriter.h      # TagLib metadata writing
│   ├── TranslationManager.h  # Runtime language switching manager
│   ├── I18n.h                # Compile-time multilingual string table
│   └── ...
├── src/               # C++ source code and QML interface
│   ├── main.cpp           # Qt application entry point
│   ├── main.qml           # Qt Quick UI (with metadata selection and write confirmation dialogs)
│   └── ...
├── tests/             # Unit tests
├── i18n/              # Translation files (.ts/.qm)
└── CMakeLists.txt     # Build configuration
```

### 5. Planned Features

- Support for more platforms (macOS, Linux, and iOS)
- Audio upsampling technology similar to Sony DSEE and audio upmix technology similar to Sony 360 Upmix
- Intelligent music-genre-aware stylized audio visualization