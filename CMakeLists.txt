cmake_minimum_required(VERSION 3.24)
project(ot_test)

find_package(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

add_executable(ot_test main.cpp gui.cpp
        gui.cpp
        gui.h)
TARGET_LINK_LIBRARIES (ot_test ${OpenCV_LIBS})