# Copyright (C) 2018 Google Inc.
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

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

# Import the ARCore library.
add_library(arcore SHARED IMPORTED)
set_target_properties(arcore PROPERTIES IMPORTED_LOCATION
                      "${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so")

add_library(sk_skia SHARED IMPORTED)
set_target_properties(sk_skia PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libarcore.so")

# This is the main app library.
add_library(hello_ar_native SHARED
           "src/main/cpp/hello_ar_application.cc"
           "src/main/cpp/background_renderer.cc"
           "src/main/cpp/jni_interface.cc"
           "src/main/cpp/plane_renderer.cc"
           "src/main/cpp/point_cloud_renderer.cc"
           "src/main/cpp/util.cc"
           "src/main/cpp/pending_anchor.cc"
           "src/main/cpp/anchor_wrapper.cc"

           "src/main/cpp/SkArCamera.cpp")

target_include_directories(hello_ar_native PRIVATE
            #BASIC AR NATIVE CODE
            "src/main/cpp"

            #ARCORE LIBRARY
            "${ARCORE_INCLUDE}"

            #GLM
            "${ANDROID_NDK}/sources/third_party/vulkan/src/libs/glm"

            #SKIA INCLUDE DIRECTORIES
            "${SKIA_INCLUDE_PATH}/../modules/skshaper/include"
            "${SKIA_INCLUDE_PATH}/../modules/skottie/include"
            "${SKIA_INCLUDE_PATH}/../tools"
            "${SKIA_INCLUDE_PATH}/../gm"
            "${SKIA_INCLUDE_PATH}/core"
            "${SKIA_INCLUDE_PATH}/config"
            "${SKIA_INCLUDE_PATH}/gpu"
            "${SKIA_INCLUDE_PATH}/android"
            "${SKIA_INCLUDE_PATH}/atlastext"
            "${SKIA_INCLUDE_PATH}/c"
            "${SKIA_INCLUDE_PATH}/codec"
            "${SKIA_INCLUDE_PATH}/effects"
            "${SKIA_INCLUDE_PATH}/encode"
            "${SKIA_INCLUDE_PATH}/pathops"
            "${SKIA_INCLUDE_PATH}/ports"
            "${SKIA_INCLUDE_PATH}/private"
            "${SKIA_INCLUDE_PATH}/svg"
            "${SKIA_INCLUDE_PATH}/utils"
            "${SKIA_INCLUDE_PATH}/views")

target_link_libraries(hello_ar_native
                      android
                      log
                      GLESv2
                      arcore
                      sk_skia)
