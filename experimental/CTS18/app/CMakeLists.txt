# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add.library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.


# Set the paths to the shared libarary and the include files.
add_library( gmrunner SHARED IMPORTED )
# set_target_properties( gmrunner PROPERTIES IMPORTED_LOCATION ../../../out/arm64/libgmrunner.so )
set_target_properties( gmrunner PROPERTIES IMPORTED_LOCATION libs/libgmrunner.so )
include_directories( ../../../gmrunner/ )

add_library( skia STATIC IMPORTED )
set_target_properties( skia PROPERTIES IMPORTED_LOCATION libs/libskia.a )

# target_link_libraries( gmrunner )
# target_link_libraries( skia )
