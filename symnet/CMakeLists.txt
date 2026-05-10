cmake_minimum_required(VERSION 3.14)
project(symnet)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 添加 utils 库
add_subdirectory(utils)

# 查找 jsoncpp 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# 1
include_directories(tcpechoserver/src)
add_executable(echo_server
        tcpechoserver/Server.cpp
)
target_link_libraries(echo_server symutils_static)

add_executable(echo_client
        tcpechoserver/Client.cpp
)
target_link_libraries(echo_client symutils_static)


# 2
include_directories(tcpcmdserver/src)
add_executable(cmd_server
        tcpcmdserver/Server.cpp
)
target_link_libraries(cmd_server symutils_static)

add_executable(cmd_client
        tcpcmdserver/Client.cpp
)
target_link_libraries(cmd_client symutils_static)

# 3 - online calculator
include_directories(online_calculator/src)
add_executable(calc_server
        online_calculator/Server.cpp
)
target_link_libraries(calc_server symutils_static ${JSONCPP_LIBRARIES})
target_include_directories(calc_server PRIVATE ${JSONCPP_INCLUDE_DIRS})

add_executable(calc_client
        online_calculator/Client.cpp
)
target_link_libraries(calc_client symutils_static ${JSONCPP_LIBRARIES})
target_include_directories(calc_client PRIVATE ${JSONCPP_INCLUDE_DIRS})