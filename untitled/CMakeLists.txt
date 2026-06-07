cmake_minimum_required(VERSION 4.2)
project(untitled)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(untitled main.cpp
        mainwindow.hpp
        dialog.hpp
)
target_link_libraries(untitled
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

