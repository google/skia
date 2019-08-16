cmake_minimum_required(VERSION 3.10)
project(skia CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(skcms "third_party/skcms/skcms.cc")
target_include_directories(skcms PRIVATE "include/third_party/skcms")

file(GLOB_RECURSE skia_sources CONFIGURE_DEPENDS
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
     src/svg/*.cpp)

list(FILTER skia_sources EXCLUDE REGEX android)
list(FILTER skia_sources EXCLUDE REGEX dawn)
list(FILTER skia_sources EXCLUDE REGEX egl)
list(FILTER skia_sources EXCLUDE REGEX glfw)
list(FILTER skia_sources EXCLUDE REGEX glx)
list(FILTER skia_sources EXCLUDE REGEX vk)

add_library(skia ${skia_sources})
target_include_directories(skia PUBLIC ".")
target_link_libraries(skia PRIVATE skcms)
