cmake_minimum_required(VERSION 3.29)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled main.cpp
        left_aileron.h
        right_aileron.h
        aileron_position.h
        stabiliser.h
        vertical.h
        horizontal.h
        FBWSystem.h
        FBWSystem.cpp
        heading.h
        heading.cpp
        left_aileron.cpp
        right_aileron.cpp     
        cardinal_direction.cpp   
)
