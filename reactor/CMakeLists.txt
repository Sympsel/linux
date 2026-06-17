cmake_minimum_required(VERSION 4.2)
project(reactor)

set(CMAKE_CXX_STANDARD 23)

include_directories(epoll utils)

add_executable(reactor Main.cpp
        Connection.hpp
        Poller.hpp
        Common.hpp
        Reactor.hpp
        Listener.cpp
        Protocol.hpp
        IOHandler.cpp
        HttpProtocol.hpp
        utils/Buffer.hpp
)
