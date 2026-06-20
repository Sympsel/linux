cmake_minimum_required(VERSION 4.2)
project(cmake_
        VERSION 1.2.3
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

add_executable(main main.cpp)

add_test(
        NAME testadd
        COMMAND main
)

include(GNUInstallDirs)
install(TARGETS main)

include(CPack)

message(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")