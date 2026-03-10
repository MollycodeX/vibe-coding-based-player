# MSCPLAYER

这是基于Gemini，ChatGPT和Claude使用vibe coding做出的个人测试项目

## 1. 介绍，用途及效果

计划为基于C++及其相关库开发的一款跨平台（Windows和Android，其余平台待定）音乐播放器，可进行暂停/播放，上一曲/下一曲和音量调节等基本功能，同时拥有风格化UI（类似Windows Aero），显示歌词，并基于音乐的状态、频率所显示的音乐可视化界面。

### 技术栈

- **语言**: C++17
- **GUI框架**: Qt 6 (Qt Quick / QML)
- **音频引擎**: [miniaudio](https://miniaud.io/) (嵌入式跨平台音频库)
- **在线元数据**: [MusicBrainz](https://musicbrainz.org/) 开源音乐数据库 API
- **在线歌词**: [LRCLIB](https://lrclib.net/) 开源歌词数据库 API
- **构建系统**: CMake (≥ 3.16)
- **协议**: MIT License

### 支持的音频格式

MP3, WAV, FLAC, OGG, AAC, WMA, M4A, Opus

## 2. 现开发阶段标准（根据开发进度实时修改）

以下功能已实现并通过测试验证：

- ✅ 基本的音频读取和操作（播放/暂停/停止/跳转/音量调节）
- ✅ 基于Qt Quick (QML)的图形用户界面
- ✅ 播放列表管理（添加文件/文件夹、移除、选择、上一曲/下一曲循环）
- ✅ 多语言支持（英文/中文）
- ✅ 联网查找歌曲元数据（通过MusicBrainz开源数据库获取艺术家、专辑、曲名）
- ✅ 联网查找歌词（通过LRCLIB开源数据库获取纯文本及同步歌词）
- ✅ UI中显示歌曲元信息（曲名、艺术家、专辑）和可折叠歌词面板
- ✅ Unicode路径支持（含CJK字符的文件路径）

## 3. 构建与测试

### 依赖

- Qt 6 (Core, Gui, Qml, Quick, QuickDialogs2, Network)
- ALSA 开发库 (Linux)
- CMake ≥ 3.16

### 构建 (Linux)

```bash
# 安装依赖
sudo apt-get install -y qt6-base-dev qt6-declarative-dev qt6-tools-dev-tools libasound2-dev

# 构建
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行测试

```bash
cd build
ctest --output-on-failure
```

## 4. 项目结构

```
MSCPLAYER/
├── include/           # C++头文件
│   ├── AudioPlayer.h      # miniaudio音频播放器（PIMPL模式）
│   ├── Playlist.h         # 播放列表管理
│   ├── PlayerController.h # QML↔C++桥接控制器
│   ├── MetadataProvider.h # MusicBrainz元数据查找
│   ├── LyricsProvider.h   # LRCLIB歌词查找
│   ├── I18n.h             # 编译时多语言字符串表
│   └── ...
├── src/               # C++源代码及QML界面
│   ├── main.cpp           # Qt应用入口
│   ├── main.qml           # Qt Quick用户界面
│   └── ...
├── tests/             # 单元测试
├── i18n/              # 翻译文件 (.ts/.qm)
└── CMakeLists.txt     # 构建配置
```

## 5. 计划中的新功能

- 支持更多平台（macOS，Linux和iOS）
- 添加类似索尼DSEE的音频上采样技术和类似索尼360 upmix的音频升格技术
- 根据所播放音乐的风格智能定制其风格化音频可视化界面
