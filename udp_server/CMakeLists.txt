cmake_minimum_required(VERSION 4.2)
project(udp_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(udp_server main.cpp)
target_link_libraries(udp_server
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

