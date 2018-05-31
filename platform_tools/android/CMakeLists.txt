cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
cmake_policy(VERSION 2.8.8)

## Supported ABIs: 'x86', 'x86_64', armeabi-v7a', 'arm64-v8a'
message("Build Type: ${CMAKE_BUILD_TYPE}")

##### ARGS.GN
## Set dev in Android Studio to true
set (android_studio "true")
set (arg_android_studio "is_androidStudio_dev = ${android_studio}")

## Set the NDK path
set (arg_ndk "ndk = \"${ANDROID_NDK}\"")

## Set target ABI
if(ANDROID_ABI STREQUAL "arm64-v8a")
    set (arg_abi "target_cpu = \"arm64\"")
elseif(ANDROID_ABI STREQUAL "x86")
    set (arg_abi "target_cpu = \"x86\"")
elseif(ANDROID_ABI STREQUAL "x86_64")
    set (arg_abi "target_cpu = \"x64\"")
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set (arg_abi "target_cpu = \"arm\"")
endif()
message ("Target ABI: ${ANDROID_ABI}")
message ("Target Builds: ${TARGETS}")

set (android_out "${CMAKE_CURRENT_SOURCE_DIR}/../../out/android_cmake_${ANDROID_ABI}")

execute_process(COMMAND mkdir -p  ${android_out}/)
execute_process(COMMAND touch  ${android_out}/args.gn)
file(WRITE "${android_out}/args.gn" "${arg_ndk}\n${arg_abi}\n${arg_android_studio}\n")

message("CMake Target Output Directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
##### GN to CMAKE
set (bin_gn "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/gn")
set (gen "gen")
message("GN Output Directory: ${android_out}")

set (ide "--ide=json")
set (script_arg "--json-ide-script=")
set (script_path "${CMAKE_CURRENT_SOURCE_DIR}/../../gn/gn_to_cmake.py")

execute_process (COMMAND ${bin_gn} ${gen} ${android_out} ${ide} ${script_arg}${script_path}
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

##### NINJA
include(${android_out}/CMakeLists.txt)