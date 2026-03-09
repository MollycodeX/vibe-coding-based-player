cmake_minimum_required(VERSION 3.15)
project(vibe-coding-based-player VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加主源码与头文件路径
include_directories("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(src)
add_subdirectory(tests)

# 可根据需要添加第三方库查找与链接操作...