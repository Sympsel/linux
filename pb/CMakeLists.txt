cmake_minimum_required(VERSION 4.2)
project(pb)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 CONFIG 模式查找 Protobuf
find_package(Protobuf CONFIG REQUIRED)

# 客户端可执行文件
add_executable(client client/main.cpp
        common/add_contact.pb.cc)
target_include_directories(client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/client
        ${CMAKE_CURRENT_SOURCE_DIR}/common
)
target_link_libraries(client
        protobuf::libprotobuf
        protobuf::libprotoc
)

# 服务端可执行文件
add_executable(server server/main.cpp
        common/add_contact.pb.cc
)
target_include_directories(server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/server
        ${CMAKE_CURRENT_SOURCE_DIR}/common
)
target_link_libraries(server
        protobuf::libprotobuf
        protobuf::libprotoc
)
