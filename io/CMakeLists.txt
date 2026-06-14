cmake_minimum_required(VERSION 4.2)
project(io)

set(CMAKE_CXX_STANDARD 23)

include_directories(epoll utils)

add_executable(io main.cpp)
