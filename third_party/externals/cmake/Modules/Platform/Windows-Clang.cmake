
#=============================================================================
# Copyright 2001-2013 Kitware, Inc.
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

# This module is shared by multiple languages; use include blocker.
if(__WINDOWS_CLANG)
  return()
endif()
set(__WINDOWS_CLANG 1)

if("x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC"
    OR "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
  include(Platform/Windows-MSVC)
  macro(__windows_compiler_clang lang)
    __windows_compiler_msvc(${lang})
  endmacro()
else()
  include(Platform/Windows-GNU)
  macro(__windows_compiler_clang lang)
    __windows_compiler_gnu(${lang})
  endmacro()
endif()
