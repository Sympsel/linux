cmake_minimum_required(VERSION 4.2)
project(http_server)

set(CMAKE_CXX_STANDARD 20)

include_directories(
        src/http
        src/tcp
        src/utils
)

add_executable(http_server Server.cpp)
