cmake_minimum_required(VERSION 2.8)
project(CompileDefinitions)

if ("${CMAKE_GENERATOR}" STREQUAL "Visual Studio 6")
    add_definitions(-DNO_SPACES_IN_DEFINE_VALUES)
    set(NO_SPACES_IN_DEFINE_VALUES 1)
endif()

# Use compile flags to tell executables which config is built
# without depending on the compile definitions functionality.
foreach(c DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set(CMAKE_C_FLAGS_${c} "${CMAKE_C_FLAGS_${c}} -DTEST_CONFIG_${c}")
  set(CMAKE_CXX_FLAGS_${c} "${CMAKE_CXX_FLAGS_${c}} -DTEST_CONFIG_${c}")
endforeach()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
  "BUILD_CONFIG_NAME=\"$<CONFIGURATION>\""
  )

add_subdirectory(add_def_cmd)
add_subdirectory(target_prop)
add_subdirectory(add_def_cmd_tprop)

add_executable(CompileDefinitions runtest.c)
