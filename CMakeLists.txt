cmake_minimum_required(VERSION 3.10)
project(skia CXX)
set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    src/core/*.cpp
    src/effects/*.cpp
    src/gpu/*.cpp
    src/image/*.cpp
    src/lazy/*.cpp
    src/opts/*.cpp
    src/pathops/*.cpp
    src/pdf/*.cpp
    src/shaders/*.cpp
    src/sksl/*.cpp
    src/svg/*.cpp
    third_party/skcms/skcms.cc)
list(FILTER sources EXCLUDE REGEX android)
list(FILTER sources EXCLUDE REGEX egl)
list(FILTER sources EXCLUDE REGEX glfw)
list(FILTER sources EXCLUDE REGEX glx)
list(FILTER sources EXCLUDE REGEX vk)

file(GLOB   public_includes include/*)
list(FILTER public_includes EXCLUDE REGEX private)

file(GLOB private_includes include/private src/* src/*/*)

add_library(skia ${sources})
target_include_directories(skia PUBLIC  ${public_includes}
                                PRIVATE ${private_includes})
