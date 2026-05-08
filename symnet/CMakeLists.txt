cmake_minimum_required(VERSION 4.2)
project(symnet)

set(CMAKE_CXX_STANDARD 17)

include_directories(lib examples/src)

add_executable(server examples/Server.cpp
        lib/src/network/SocketUtil.cpp)
add_executable(client examples/Client.cpp)
#add_executable(udp_server lib/include/network/UdpSocket.hpp
#        lib/include/network/UdpSocket.hpp)
