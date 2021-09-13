cmake_minimum_required(VERSION 3.20)
project(lowp_basic)

set(CMAKE_CXX_STANDARD 17)

add_executable(lowp lowp_experiments.cpp)

add_executable(lerp lerp-study.cpp)

add_executable(bilerp bilerp-study.cpp)
