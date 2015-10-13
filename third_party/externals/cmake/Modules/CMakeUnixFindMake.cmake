
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

find_program(CMAKE_MAKE_PROGRAM NAMES gmake make smake)
mark_as_advanced(CMAKE_MAKE_PROGRAM)

# Look for a make tool provided by Xcode
if(NOT CMAKE_MAKE_PROGRAM AND CMAKE_HOST_APPLE)
  execute_process(COMMAND xcrun --find make
    OUTPUT_VARIABLE _xcrun_out OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE _xcrun_err)
  if(_xcrun_out)
    set_property(CACHE CMAKE_MAKE_PROGRAM PROPERTY VALUE "${_xcrun_out}")
  endif()
endif()
