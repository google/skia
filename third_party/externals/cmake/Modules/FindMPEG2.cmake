#.rst:
# FindMPEG2
# ---------
#
# Find the native MPEG2 includes and library
#
# This module defines
#
# ::
#
#   MPEG2_INCLUDE_DIR, path to mpeg2dec/mpeg2.h, etc.
#   MPEG2_LIBRARIES, the libraries required to use MPEG2.
#   MPEG2_FOUND, If false, do not try to use MPEG2.
#
# also defined, but not for general use are
#
# ::
#
#   MPEG2_mpeg2_LIBRARY, where to find the MPEG2 library.
#   MPEG2_vo_LIBRARY, where to find the vo library.

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

find_path(MPEG2_INCLUDE_DIR
  NAMES mpeg2.h mpeg2dec/mpeg2.h
  PATHS /usr/local/livid
)

find_library(MPEG2_mpeg2_LIBRARY mpeg2
  /usr/local/livid/mpeg2dec/libmpeg2/.libs
)

find_library( MPEG2_vo_LIBRARY vo
  /usr/local/livid/mpeg2dec/libvo/.libs
)


# handle the QUIETLY and REQUIRED arguments and set MPEG2_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MPEG2 DEFAULT_MSG MPEG2_mpeg2_LIBRARY MPEG2_INCLUDE_DIR)

if(MPEG2_FOUND)
  set( MPEG2_LIBRARIES ${MPEG2_mpeg2_LIBRARY}
                        ${MPEG2_vo_LIBRARY})

  #some native mpeg2 installations will depend
  #on libSDL, if found, add it in.
  include(${CMAKE_CURRENT_LIST_DIR}/FindSDL.cmake)
  if(SDL_FOUND)
    set( MPEG2_LIBRARIES ${MPEG2_LIBRARIES} ${SDL_LIBRARY})
  endif()
endif()

mark_as_advanced(MPEG2_INCLUDE_DIR MPEG2_mpeg2_LIBRARY MPEG2_vo_LIBRARY)
