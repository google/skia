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
# set_target_properties( gmrunner PROPERTIES IMPORTED_LOCATION ../../../out/${ANDROID_ABI}/libgmrunner.so )
#set_target_properties( gmrunner PROPERTIES IMPORTED_LOCATION libs/libgmrunner.so )

set_target_properties( gmrunner
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_CURRENT_LIST_DIR}/libs/${ANDROID_ABI}/libgmrunner.so )
include_directories( /usr/local/google/home/stephana/gospace/src/skia.googlesource.com/skia/experimental/gmrunner/ )

add_library( skia SHARED IMPORTED )
set_target_properties( skia
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_CURRENT_LIST_DIR}/libs/${ANDROID_ABI}/libskia.a )
include_directories( /usr/local/google/home/stephana/gospace/src/skia.googlesource.com/skia/out/${ANDROID_ABI}/gen/ )

add_library( cts18jni SHARED src/main/cpp/gmrunner_jni.cpp )
include_directories( src/main/cpp/ )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(LINK_FLAGS ${LINK_FLAGS} "-Wl,-whole-archive")

target_link_libraries( cts18jni skia gmrunner skia )
# target_link_libraries( skia )
