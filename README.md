# MSCPLAYER

这是基于 Gemini、ChatGPT 和 Claude 使用 vibe coding 做出的个人测试项目

## 介绍、用途及效果

计划为基于 C++ 及其相关库开发的一款跨平台（Windows 和 Android，其余平台待定）音乐播放器，可进行暂停/播放、上一曲/下一曲和音量调节等基本功能，同时拥有风格化 UI（类似 Windows Aero），显示歌词，并基于音乐的状态、频率所显示的音乐可视化界面。

## 现开发阶段标准（根据开发进度实时修改）

已实现基本的音频读取和操作，已加上基于 Qt Quick (QML) 的 GUI（含播放列表管理）和多语言支持（英文/中文），正在尝试寻找开源数据库以联网查找歌曲元数据及歌词。

## 计划中的新功能

- 支持更多平台（macOS、Linux 和 iOS）
- 支持联网查找音频文件的元数据及其歌词
- 添加类似索尼 DSEE 的音频上采样技术和类似索尼 360 upmix 的音频升格技术
- 根据所播放音乐的风格智能定制其风格化音频可视化界面

## 构建方法

### 依赖

- CMake 3.16+
- Qt 6（Core、Gui、Qml、Quick、QuickDialogs2）
- C++17 编译器
- Linux：`libasound2-dev`

### 构建步骤

```bash
# 安装依赖（Ubuntu/Debian）
sudo apt-get install -y qt6-base-dev qt6-declarative-dev qt6-tools-dev-tools libasound2-dev

# 使用 CMake Presets 构建
cmake --preset default
cmake --build build

# 或手动构建
mkdir build && cd build
cmake ..
cmake --build .
```

### 运行测试

```bash
ctest --test-dir build --output-on-failure
```

## 开发标准

本项目遵循以下开发标准，通过配置文件自动执行：

| 标准 | 工具 | 配置文件 |
|------|------|----------|
| 代码格式化 | clang-format | `.clang-format` |
| 静态分析 | clang-tidy | `.clang-tidy` |
| 编辑器一致性 | EditorConfig | `.editorconfig` |
| 持续集成 | GitHub Actions | `.github/workflows/ci.yml` |
| 构建配置 | CMake Presets | `CMakePresets.json` |

### 编码规范

- **语言标准**：C++17
- **缩进**：4 个空格（禁用 Tab）
- **行宽上限**：100 字符
- **文件编码**：UTF-8，LF 行尾
- **花括号风格**：K&R（Attach）
- **命名约定**：类名 PascalCase，方法名 camelCase，成员变量 m_ 前缀

### CI 流程

每次推送至 `main` 或发起 Pull Request 时，GitHub Actions 会自动执行：

1. 安装依赖
2. 配置并编译项目
3. 运行单元测试
