
#=============================================================================
# Copyright 2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if(NOT DEFINED CMAKE_Fortran_COMPILER)
  set(_desc "Looking for a Fortran compiler")
  message(STATUS ${_desc})
  file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckFortran)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckFortran/CMakeLists.txt"
    "cmake_minimum_required(VERSION 2.4)
project(CheckFortran Fortran)
file(WRITE \"\${CMAKE_CURRENT_BINARY_DIR}/result.cmake\"
  \"set(CMAKE_Fortran_COMPILER \\\"\${CMAKE_Fortran_COMPILER}\\\")\\n\"
  \"set(CMAKE_Fortran_FLAGS \\\"\${CMAKE_Fortran_FLAGS}\\\")\\n\"
  )
")
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckFortran
    COMMAND ${CMAKE_COMMAND} . -G ${CMAKE_GENERATOR}
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    RESULT_VARIABLE result
    )
  include(${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CheckFortran/result.cmake OPTIONAL)
  if(CMAKE_Fortran_COMPILER AND "${result}" STREQUAL "0")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "${_desc} passed with the following output:\n"
      "${output}\n")
  else()
    set(CMAKE_Fortran_COMPILER NOTFOUND)
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "${_desc} failed with the following output:\n"
      "${output}\n")
  endif()
  message(STATUS "${_desc} - ${CMAKE_Fortran_COMPILER}")
  set(CMAKE_Fortran_COMPILER "${CMAKE_Fortran_COMPILER}" CACHE FILEPATH "Fortran compiler")
  mark_as_advanced(CMAKE_Fortran_COMPILER)
  set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS}" CACHE STRING "Fortran flags")
  mark_as_advanced(CMAKE_Fortran_FLAGS)
endif()
