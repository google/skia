cmake_minimum_required (VERSION 2.6)
project(Plugin)

# Test per-target output directory properties.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/plugin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Plugin_BINARY_DIR}/lib/static)

# We need the dynamic loader support from KWSys to load the plugin in
# the executable.
set(KWSYS_NAMESPACE kwsys)
set(KWSYS_HEADER_ROOT ${Plugin_BINARY_DIR}/include)
set(KWSYS_USE_DynamicLoader 1)
add_subdirectory(${Plugin_SOURCE_DIR}/../../Source/kwsys src/kwsys)

# Configure the location of plugins.
configure_file(${Plugin_SOURCE_DIR}/src/example_exe.h.in
               ${Plugin_BINARY_DIR}/include/example_exe.h @ONLY)

# We need to include headers from the source tree and configured
# headers in the build tree.
include_directories(
  ${Plugin_BINARY_DIR}/include
  ${Plugin_SOURCE_DIR}/include
  )

# We need proper C++98 support from the compiler
set(CMAKE_CXX_STANDARD 98)

# Those versions of the HP compiler that need a flag to get proper C++98
# template support also need a flag to use the newer C++ library.
if (CMAKE_CXX_COMPILER_ID STREQUAL HP AND
    CMAKE_CXX98_STANDARD_COMPILE_OPTION STREQUAL "+hpxstd98")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AA")
endif ()

# Create an executable that exports an API for use by plugins.
add_executable(example_exe src/example_exe.cxx)
set_target_properties(example_exe PROPERTIES
  ENABLE_EXPORTS 1
  OUTPUT_NAME example
  # Test placing exe import library in unique directory.
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/exe
  )
target_link_libraries(example_exe kwsys)

# Create a plugin that uses the API provided by the executable.
# This module "links" to the executable to use the symbols.
add_library(example_mod_1 MODULE src/example_mod_1.c)
target_link_libraries(example_mod_1 example_exe)


if(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG AND
    "${CMAKE_C_CREATE_SHARED_MODULE}" MATCHES "SONAME_FLAG")
  # Add a second plugin that should not have any soname.
  add_library(example_mod_2 MODULE src/example_mod_1.c)
  target_link_libraries(example_mod_2 example_exe)
  set_property(TARGET example_mod_2 PROPERTY NO_SONAME 1)

  # Verify that targets export with proper IMPORTED SONAME properties.
  export(TARGETS example_mod_1 example_mod_2 NAMESPACE exp_
    FILE ${CMAKE_CURRENT_BINARY_DIR}/mods.cmake)

  include(ExternalProject)
  ExternalProject_Add(PluginTest
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PluginTest"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/PluginTest"
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
  )
  add_dependencies(PluginTest example_mod_1 example_mod_2)
endif()

# TODO:
#  - create a plugin that links to a static lib
#  - create a plugin that links to a shared lib
