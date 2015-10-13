
#=============================================================================
# Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>
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

if(__GHSMULTI)
  return()
endif()
set(__GHSMULTI 1)

set(GHSMULTI 1)

set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

include(Platform/WindowsPaths)
