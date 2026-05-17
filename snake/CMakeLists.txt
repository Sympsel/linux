cmake_minimum_required(VERSION 4.2)
project(snake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找 ncurses 库
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

#add_executable(
#        snake src/main.cpp
#)

# 链接必要的库
target_link_libraries(snake ${CURSES_LIBRARIES} jsoncpp pthread)

add_executable(
        launch launch.cpp
)