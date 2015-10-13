#.rst:
# TestForSTDNamespace
# -------------------
#
# Test for std:: namespace support
#
# check if the compiler supports std:: on stl classes
#
# ::
#
#   CMAKE_NO_STD_NAMESPACE - defined by the results

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

if(NOT DEFINED CMAKE_STD_NAMESPACE)
  message(STATUS "Check for STD namespace")
  try_compile(CMAKE_STD_NAMESPACE  ${CMAKE_BINARY_DIR}
    ${CMAKE_ROOT}/Modules/TestForSTDNamespace.cxx
    OUTPUT_VARIABLE OUTPUT)
  if (CMAKE_STD_NAMESPACE)
    message(STATUS "Check for STD namespace - found")
    set (CMAKE_NO_STD_NAMESPACE 0 CACHE INTERNAL
         "Does the compiler support std::.")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the CXX compiler has std namespace passed with "
      "the following output:\n${OUTPUT}\n\n")
  else ()
    message(STATUS "Check for STD namespace - not found")
    set (CMAKE_NO_STD_NAMESPACE 1 CACHE INTERNAL
       "Does the compiler support std::.")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determining if the CXX compiler has std namespace failed with "
      "the following output:\n${OUTPUT}\n\n")
  endif ()
endif()




