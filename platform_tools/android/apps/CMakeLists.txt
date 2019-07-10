cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
cmake_policy(VERSION 2.8.8)

## List out all the properties (useful for debugging!)
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

##### ARGS.GN Setup

## Set the NDK path, NDK API version level, and target ABI
set (arg_ndk "ndk=\"${ANDROID_NDK}\"\n")
set (arg_ndk_api "ndk_api=${ANDROID_NATIVE_API_LEVEL}\n")
set (arg_abi "target_cpu=\"${ANDROID_ARCH_NAME}\"\n")

# Use Vulkan if API > 24
set (arg_vulkan)

# Set Release vs Debug
set (arg_debug "")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set (arg_debug "is_debug=false\n")
endif()

# Setup output directory
set (android_out "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/android_cmake_${ANDROID_ARCH_NAME}")
message("GN Output Directory: ${android_out}")
message("CMake Target Output Directory: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

execute_process(COMMAND mkdir -p  ${android_out}/)
execute_process(COMMAND touch  ${android_out}/args.gn)
file(WRITE "${android_out}/args.gn" "${arg_debug}${arg_ndk}${arg_ndk_api}${arg_abi}")

##### GN to CMAKE
set (bin_gn "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/gn")
set (gen "gen")

set (ide "--ide=json")
set (script_arg "--json-ide-script=")
set (script_path "${CMAKE_CURRENT_SOURCE_DIR}/../../../gn/gn_to_cmake.py")

execute_process (COMMAND ${bin_gn} ${gen} ${android_out} ${ide} ${script_arg}${script_path}
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" RESULT_VARIABLE retcode)

if(NOT "${retcode}" STREQUAL "0")
message(FATAL_ERROR "Fatal error when running GN in ${android_out}")
endif()


##### NINJA
string(REPLACE "/ninja" "" ninja_path "${CMAKE_MAKE_PROGRAM}")
message("Ninja Path: ${ninja_path}")
set(ENV{PATH} $ENV{PATH}:${ninja_path})

include(${android_out}/CMakeLists.txt)