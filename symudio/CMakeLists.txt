cmake_minimum_required(VERSION 4.2)
project(symudio)

set(CMAKE_CXX_STANDARD 23)

include_directories(utils entity)

add_executable(symudio-server Server.cpp
        entity/Any.hpp
        entity/Buffer.hpp
        utils/Log.hpp
        entity/Socket.hpp
        entity/Poller.cpp
        entity/Poller.h
        entity/Channel.cpp
        entity/Channel.h
        entity/EventLoop.cpp
        entity/EventLoop.h
        entity/TimerWheel.h
        entity/TimerWheel.cpp
        entity/Connection.hpp
)

add_executable(symudio-client Client.cpp
        entity/Any.hpp
        entity/Buffer.hpp
        utils/Log.hpp
        entity/Socket.hpp
)
