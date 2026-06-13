cmake_minimum_required(VERSION 4.2)
project(io)

set(CMAKE_CXX_STANDARD 23)

add_executable(io main.cpp
        InetAddr.hpp
        Log.hpp
        SelectServer.hpp
        TcpServer.hpp)
