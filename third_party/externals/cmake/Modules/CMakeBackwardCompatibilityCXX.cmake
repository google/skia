#.rst:
# CMakeBackwardCompatibilityCXX
# -----------------------------
#
# define a bunch of backwards compatibility variables
#
# ::
#
#   CMAKE_ANSI_CXXFLAGS - flag for ansi c++
#   CMAKE_HAS_ANSI_STRING_STREAM - has <strstream>
#   include(TestForANSIStreamHeaders)
#   include(CheckIncludeFileCXX)
#   include(TestForSTDNamespace)
#   include(TestForANSIForScope)

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

if(NOT CMAKE_SKIP_COMPATIBILITY_TESTS)
  # check for some ANSI flags in the CXX compiler if it is not gnu
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    include(TestCXXAcceptsFlag)
    set(CMAKE_TRY_ANSI_CXX_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "IRIX")
      set(CMAKE_TRY_ANSI_CXX_FLAGS "-LANG:std")
    endif()
    if(CMAKE_SYSTEM_NAME MATCHES "OSF")
      set(CMAKE_TRY_ANSI_CXX_FLAGS "-std strict_ansi -nopure_cname")
    endif()
    # if CMAKE_TRY_ANSI_CXX_FLAGS has something in it, see
    # if the compiler accepts it
    if(NOT CMAKE_TRY_ANSI_CXX_FLAGS STREQUAL "")
      CHECK_CXX_ACCEPTS_FLAG(${CMAKE_TRY_ANSI_CXX_FLAGS} CMAKE_CXX_ACCEPTS_FLAGS)
      # if the compiler liked the flag then set CMAKE_ANSI_CXXFLAGS
      # to the flag
      if(CMAKE_CXX_ACCEPTS_FLAGS)
        set(CMAKE_ANSI_CXXFLAGS ${CMAKE_TRY_ANSI_CXX_FLAGS} CACHE INTERNAL
        "What flags are required by the c++ compiler to make it ansi." )
      endif()
    endif()
  endif()
  set(CMAKE_CXX_FLAGS_SAVE ${CMAKE_CXX_FLAGS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")
  include(TestForANSIStreamHeaders)
  include(CheckIncludeFileCXX)
  include(TestForSTDNamespace)
  include(TestForANSIForScope)
  include(TestForSSTREAM)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE}")
endif()

