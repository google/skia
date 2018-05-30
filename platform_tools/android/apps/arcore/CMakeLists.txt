# Copyright (C) 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Import the ARCore library.
add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION
                      ${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so)

add_library(skia SHARED IMPORTED)
set_target_properties(skia PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libskia.so)

# This is the main app library.
add_library(hello_ar_native SHARED
           src/main/cpp/background_renderer.cc
           src/main/cpp/hello_ar_application.cc
           src/main/cpp/jni_interface.cc
           src/main/cpp/obj_renderer.cc
           src/main/cpp/plane_renderer.cc
           src/main/cpp/point_cloud_renderer.cc
           src/main/cpp/util.cc)

target_include_directories(hello_ar_native PRIVATE
            src/main/cpp
            ${ARCORE_INCLUDE}
            ${ANDROID_NDK}/sources/third_party/vulkan/src/libs/glm

            #SKIA INCLUDE DIRECTORIES
            /usr/local/google/home/ziadb/skia/include/core
            /usr/local/google/home/ziadb/skia/include/config
            /usr/local/google/home/ziadb/skia/include/gpu
            /usr/local/google/home/ziadb/skia/include/gpu/gl
            /usr/local/google/home/ziadb/skia/include/android
            /usr/local/google/home/ziadb/skia/include/atlastext
            /usr/local/google/home/ziadb/skia/include/c
            /usr/local/google/home/ziadb/skia/include/codec
            /usr/local/google/home/ziadb/skia/include/effects
            /usr/local/google/home/ziadb/skia/include/encode
            /usr/local/google/home/ziadb/skia/include/pathops
            /usr/local/google/home/ziadb/skia/include/ports
            /usr/local/google/home/ziadb/skia/include/private
            /usr/local/google/home/ziadb/skia/include/svg
            /usr/local/google/home/ziadb/skia/include/utils
            /usr/local/google/home/ziadb/skia/include/views
            )

target_link_libraries(hello_ar_native
                      android
                      log
                      GLESv2
                      arcore
                      skia)
