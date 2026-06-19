cmake_minimum_required(VERSION 4.2)

project(TestInclude)

# 打印当前正在执行的源代码目录
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

# 打印当前正在执行的CMake脚本的完整名称
message(STATUS ${CMAKE_CURRENT_LIST_FILE})
message(STATUS ${CMAKE_CURRENT_LIST_DIR})

# 包含子目录cmake脚本
include(sub/sub.cmake)
