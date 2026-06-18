cmake_minimum_required(VERSION 4.2)
project(reactor)

set(CMAKE_CXX_STANDARD 23)

include_directories(epoll utils core network http)

add_executable(reactor Main.cpp
        network/Listener.cpp
        http/HttpConnection.cpp
)
