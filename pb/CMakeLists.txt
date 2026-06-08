cmake_minimum_required(VERSION 4.2)
project(pb)

set(CMAKE_CXX_STANDARD 20)

# 尝试使用 CONFIG 模式,如果失败则使用模块模式
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    find_package(Protobuf MODULE REQUIRED)
endif()

add_executable(pb main.cpp
        contacts.pb.cc)

# 根据找到的 Protobuf 类型选择正确的 target
if(TARGET protobuf::libprotobuf)
    target_link_libraries(pb protobuf::libprotobuf)
else()
    target_link_libraries(pb ${PROTOBUF_LIBRARIES})
    # 查找并链接 absl 库
    find_library(ABSL_LOG_INTERNAL_CHECK_OP NAMES absl_log_internal_check_op)
    find_library(ABSL_STRINGS NAMES absl_strings)
    if(ABSL_LOG_INTERNAL_CHECK_OP)
        target_link_libraries(pb ${ABSL_LOG_INTERNAL_CHECK_OP})
    endif()
    if(ABSL_STRINGS)
        target_link_libraries(pb ${ABSL_STRINGS})
    endif()
endif()

target_include_directories(pb PRIVATE ${PROTOBUF_INCLUDE_DIRS})

#  g++ -std=c++20 -o main main.cpp contacts.pb.cc $(pkg-config --cflags --libs protobuf)
