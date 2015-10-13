#.rst:
# TestForANSIForScope
# -------------------
#
# Check for ANSI for scope support
#
# Check if the compiler restricts the scope of variables declared in a
# for-init-statement to the loop body.
#
# ::
#
#   CMAKE_NO_ANSI_FOR_SCOPE - holds result

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

if(NOT DEFINED CMAKE_ANSI_FOR_SCOPE)
  message(STATUS "Check for ANSI scope")
  try_compile(CMAKE_ANSI_FOR_SCOPE  ${CMAKE_BINARY_DIR}
    ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx
    OUTPUT_VARIABLE OUTPUT)
  if (CMAKE_ANSI_FOR_SCOPE)
    message(STATUS "Check for ANSI scope - found")
    set (CMAKE_NO_ANSI_FOR_SCOPE 0 CACHE INTERNAL
      "Does the compiler support ansi for scope.")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the CXX compiler understands ansi for scopes passed with "
      "the following output:\n${OUTPUT}\n\n")
  else ()
    message(STATUS "Check for ANSI scope - not found")
    set (CMAKE_NO_ANSI_FOR_SCOPE 1 CACHE INTERNAL
      "Does the compiler support ansi for scope.")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determining if the CXX compiler understands ansi for scopes failed with "
      "the following output:\n${OUTPUT}\n\n")
  endif ()
endif()





