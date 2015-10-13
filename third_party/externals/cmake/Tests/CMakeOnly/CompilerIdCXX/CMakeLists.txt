cmake_minimum_required(VERSION 2.8.9)
project(CompilerIdCXX CXX)

foreach(v
    CMAKE_CXX_COMPILER
    CMAKE_CXX_COMPILER_ID
    CMAKE_CXX_COMPILER_VERSION
    )
  if(${v})
    message(STATUS "${v}=[${${v}}]")
  else()
    message(SEND_ERROR "${v} not set!")
  endif()
endforeach()
