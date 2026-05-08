cmake_minimum_required(VERSION 4.2)
project(013_udpechoserver)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)

add_executable(server TcpEchoServer.cpp
        src/Cond.hpp
        src/Mutex.hpp
        src/Log.hpp
        src/Thread.hpp
        src/ThreadPool.hpp
        src/TcpEchoServer.hpp
        src/TcpSocket.hpp
        src/InetAddr.hpp
        TcpEchoClient.cpp)
add_executable(client TcpEchoClient.cpp
        src/TcpSocket.hpp
)
