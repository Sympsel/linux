include_directories(utils/include utils/module utils/src)
cmake_minimum_required(VERSION 3.14)
project(symutils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 收集所有源文件
set(SLOG_SOURCES
    src/slog/Log.cpp
    src/slog/LogMsg.cpp
    src/slog/Logger.cpp
    src/slog/LogStrategyVirtual.cpp
    src/slog/ConsoleLogStrategy.cpp
    src/slog/FileLogStrategy.cpp
    src/slog/CombinedLogStrategy.cpp
)

set(NETWORK_SOURCES
    src/network/SocketUtils.cpp
)

# 可以选择创建静态库或动态库
# 选项1: 静态库 (.a)
add_library(symutils_static STATIC
    ${SLOG_SOURCES}
    ${NETWORK_SOURCES}
)

# 选项2: 动态库 (.so) - 如果需要可以取消注释
# add_library(symutils_shared SHARED
#     ${SLOG_SOURCES}
#     ${NETWORK_SOURCES}
# )

# 设置头文件搜索路径（PUBLIC 表示使用该库的目标也会继承这些路径）
target_include_directories(symutils_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 如果使用动态库，也设置它
# target_include_directories(symutils_shared PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# 可选：设置库的输出名称
set_target_properties(symutils_static PROPERTIES
    OUTPUT_NAME symutils
    CLEAN_DIRECT_OUTPUT 1
)

# 如果有依赖的库，在这里链接
# target_link_libraries(symutils_static pthread)

# 打印信息用于调试
message(STATUS "Building symutils library")
message(STATUS "Include directory: ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Sources: ${SLOG_SOURCES} ${NETWORK_SOURCES}")
