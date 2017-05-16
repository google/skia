#.rst:
# CheckLanguage
# -------------
#
# Check if a language can be enabled
#
# Usage:
#
# ::
#
#   check_language(<lang>)
#
# where <lang> is a language that may be passed to enable_language()
# such as "Fortran".  If CMAKE_<lang>_COMPILER is already defined the
# check does nothing.  Otherwise it tries enabling the language in a
# test project.  The result is cached in CMAKE_<lang>_COMPILER as the
# compiler that was found, or NOTFOUND if the language cannot be
# enabled.
#
# Example:
#
# ::
#
#   check_language(Fortran)
#   if(CMAKE_Fortran_COMPILER)
#     enable_language(Fortran)
#   else()
#     message(STATUS "No Fortran support")
#   endif()

#=============================================================================
# Copyright 2009-2012 Kitware, Inc.
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

macro(check_language lang)
  if(NOT DEFINED CMAKE_${lang}_COMPILER)
    set(_desc "Looking for a ${lang} compiler")
    message(STATUS ${_desc})
    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang})
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang}/CMakeLists.txt"
      "cmake_minimum_required(VERSION ${CMAKE_VERSION})
project(Check${lang} ${lang})
file(WRITE \"\${CMAKE_CURRENT_BINARY_DIR}/result.cmake\"
  \"set(CMAKE_${lang}_COMPILER \\\"\${CMAKE_${lang}_COMPILER}\\\")\\n\"
  )
")
    execute_process(
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang}
      COMMAND ${CMAKE_COMMAND} . -G ${CMAKE_GENERATOR}
      OUTPUT_VARIABLE output
      ERROR_VARIABLE output
      RESULT_VARIABLE result
      )
    include(${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Check${lang}/result.cmake OPTIONAL)
    if(CMAKE_${lang}_COMPILER AND "${result}" STREQUAL "0")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "${_desc} passed with the following output:\n"
        "${output}\n")
    else()
      set(CMAKE_${lang}_COMPILER NOTFOUND)
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "${_desc} failed with the following output:\n"
        "${output}\n")
    endif()
    message(STATUS "${_desc} - ${CMAKE_${lang}_COMPILER}")
    set(CMAKE_${lang}_COMPILER "${CMAKE_${lang}_COMPILER}" CACHE FILEPATH "${lang} compiler")
    mark_as_advanced(CMAKE_${lang}_COMPILER)
  endif()
endmacro()
