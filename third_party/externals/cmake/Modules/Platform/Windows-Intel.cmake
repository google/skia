
#=============================================================================
# Copyright 2002-2010 Kitware, Inc.
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
if(__WINDOWS_INTEL)
  return()
endif()
set(__WINDOWS_INTEL 1)

include(Platform/Windows-MSVC)
macro(__windows_compiler_intel lang)
  __windows_compiler_msvc(${lang})
  string(REPLACE "<CMAKE_LINKER> /lib" "lib" CMAKE_${lang}_CREATE_STATIC_LIBRARY "${CMAKE_${lang}_CREATE_STATIC_LIBRARY}")
  foreach(rule CREATE_SHARED_LIBRARY CREATE_SHARED_MODULE LINK_EXECUTABLE)
    string(REPLACE "<CMAKE_LINKER>" "xilink" CMAKE_${lang}_${rule} "${CMAKE_${lang}_${rule}}")
  endforeach()
endmacro()
