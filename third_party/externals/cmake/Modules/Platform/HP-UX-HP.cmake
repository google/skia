
#=============================================================================
# Copyright 2002-2011 Kitware, Inc.
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
if(__HPUX_COMPILER_HP)
  return()
endif()
set(__HPUX_COMPILER_HP 1)

macro(__hpux_compiler_hp lang)
  set(CMAKE_${lang}_COMPILE_OPTIONS_PIC "+Z")
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS "+Z")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-Wl,-E,+nodefaultrpath -b -L/usr/lib")
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-Wl,+s,-E,+nodefaultrpath")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG "-Wl,+b")
  set(CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG_SEP ":")
  set(CMAKE_SHARED_LIBRARY_SONAME_${lang}_FLAG "-Wl,+h")

  set(CMAKE_${lang}_FLAGS_INIT "")
endmacro()
