
#=============================================================================
# Copyright 2010 Kitware, Inc.
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

function(PrintTestCompilerStatus LANG MSG)
  if(CMAKE_GENERATOR MATCHES Make)
    message(STATUS "Check for working ${LANG} compiler: ${CMAKE_${LANG}_COMPILER}${MSG}")
  else()
    message(STATUS "Check for working ${LANG} compiler using: ${CMAKE_GENERATOR}${MSG}")
  endif()
endfunction()
