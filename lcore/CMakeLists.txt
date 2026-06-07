cmake_minimum_required(VERSION 4.2)
project(lcore)

include_directories("/usr/lib/modules/7.0.9-arch1-1/build/include")
set(CMAKE_CXX_STANDARD 20)

add_executable(lcore main.c)
