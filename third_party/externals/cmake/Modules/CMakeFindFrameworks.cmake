#.rst:
# CMakeFindFrameworks
# -------------------
#
# helper module to find OSX frameworks

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
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

if(NOT CMAKE_FIND_FRAMEWORKS_INCLUDED)
  set(CMAKE_FIND_FRAMEWORKS_INCLUDED 1)
  macro(CMAKE_FIND_FRAMEWORKS fwk)
    set(${fwk}_FRAMEWORKS)
    if(APPLE)
      foreach(dir
          ~/Library/Frameworks/${fwk}.framework
          /Library/Frameworks/${fwk}.framework
          /System/Library/Frameworks/${fwk}.framework
          /Network/Library/Frameworks/${fwk}.framework)
        if(EXISTS ${dir})
          set(${fwk}_FRAMEWORKS ${${fwk}_FRAMEWORKS} ${dir})
        endif()
      endforeach()
    endif()
  endmacro()
endif()
