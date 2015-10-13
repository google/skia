cmake_minimum_required(VERSION 2.8)
project(VSMidl)

if(MSVC_VERSION GREATER 1200)
  include_directories("${CMAKE_CURRENT_BINARY_DIR}/\$(IntDir)")
else()
  # midl generated headers end up directly in CMAKE_CURRENT_BINARY_DIR with
  # VS6 builds.
  include_directories("${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_executable(VSMidl main.cpp test.idl)
