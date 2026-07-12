cmake_minimum_required(VERSION 4.2)
project(rpc)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

include_directories(${JSONCPP_INCLUDE_DIRS} utils)

add_executable(demo-jsoncpp
        demo/jsoncpp/jsoncpp.cpp
        utils/JsonTrans.hpp
)
target_link_libraries(demo-jsoncpp ${JSONCPP_LIBRARIES})

add_executable(rpc main.cpp
        utils/JsonTrans.hpp
)

target_link_libraries(rpc ${JSONCPP_LIBRARIES})

