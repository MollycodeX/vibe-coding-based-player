cmake_minimum_required(VERSION 3.16)
project(vibe-coding-based-player VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ---------------------------------------------------------------------------
# Qt6 GUI / QML / i18n dependencies
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

# ---------------------------------------------------------------------------
# 添加主源码与头文件路径
# ---------------------------------------------------------------------------
include_directories("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
