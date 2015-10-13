#.rst:
# FindMPEG
# --------
#
# Find the native MPEG includes and library
#
# This module defines
#
# ::
#
#   MPEG_INCLUDE_DIR, where to find MPEG.h, etc.
#   MPEG_LIBRARIES, the libraries required to use MPEG.
#   MPEG_FOUND, If false, do not try to use MPEG.
#
# also defined, but not for general use are
#
# ::
#
#   MPEG_mpeg2_LIBRARY, where to find the MPEG library.
#   MPEG_vo_LIBRARY, where to find the vo library.

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

find_path(MPEG_INCLUDE_DIR mpeg2dec/include/video_out.h
  /usr/local/livid
)

find_library(MPEG_mpeg2_LIBRARY mpeg2
  /usr/local/livid/mpeg2dec/libmpeg2/.libs
)

find_library( MPEG_vo_LIBRARY vo
  /usr/local/livid/mpeg2dec/libvo/.libs
)

# handle the QUIETLY and REQUIRED arguments and set MPEG2_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MPEG DEFAULT_MSG MPEG_INCLUDE_DIR MPEG_mpeg2_LIBRARY MPEG_vo_LIBRARY)

if(MPEG_FOUND)
  set( MPEG_LIBRARIES ${MPEG_mpeg2_LIBRARY} ${MPEG_vo_LIBRARY} )
endif()

mark_as_advanced(MPEG_INCLUDE_DIR MPEG_mpeg2_LIBRARY MPEG_vo_LIBRARY)
