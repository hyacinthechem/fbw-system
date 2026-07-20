cmake_minimum_required(VERSION 3.29)
project(fbw-system)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled
        src/main.cpp
        src/left_aileron.cpp
        src/right_aileron.cpp
        src/heading.cpp
        src/FBWSystem.cpp
        src/cardinal_direction.cpp
)

target_include_directories(fbw-system PRIVATE include)