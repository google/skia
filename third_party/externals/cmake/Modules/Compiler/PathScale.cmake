
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
if(__COMPILER_PATHSCALE)
  return()
endif()
set(__COMPILER_PATHSCALE 1)

macro(__compiler_pathscale lang)
  # Feature flags.
  set(CMAKE_${lang}_VERBOSE_FLAG "-v")

  # Initial configuration flags.
  set(CMAKE_${lang}_FLAGS_INIT "")
  set(CMAKE_${lang}_FLAGS_DEBUG_INIT "-g -O0")
  set(CMAKE_${lang}_FLAGS_MINSIZEREL_INIT "-Os")
  set(CMAKE_${lang}_FLAGS_RELEASE_INIT "-O3")
  set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT "-g -O2")
endmacro()
