cmake_minimum_required(VERSION 4.2)
project(012_chatroom)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(server
        src/UdpSocket.hpp
        src/Log.hpp
        chatServer.cpp
        src/RouteServer.hpp
        src/UdpServer.hpp
        src/User.hpp
        src/Cond.hpp
        src/Thread.hpp
        src/ThreadPool.hpp
        src/IniParse.hpp
        src/Config.hpp
        src/InetAddr.hpp
        src/CallbackTypes.hpp)

add_executable(client
        chatClient.cpp
        src/UdpSocket.hpp
        src/Log.hpp
        src/ChatClient.hpp
        src/IniParse.hpp
        src/Config.hpp
        src/InetAddr.hpp
        src/CallbackTypes.hpp
)