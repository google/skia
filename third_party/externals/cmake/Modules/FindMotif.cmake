#.rst:
# FindMotif
# ---------
#
# Try to find Motif (or lesstif)
#
# Once done this will define:
#
# ::
#
#   MOTIF_FOUND        - system has MOTIF
#   MOTIF_INCLUDE_DIR  - include paths to use Motif
#   MOTIF_LIBRARIES    - Link these to use Motif

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
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

set(MOTIF_FOUND 0)

if(UNIX)
  find_path(MOTIF_INCLUDE_DIR
    Xm/Xm.h
    /usr/openwin/include
    )

  find_library(MOTIF_LIBRARIES
    Xm
    /usr/openwin/lib
    )

endif()

# handle the QUIETLY and REQUIRED arguments and set MOTIF_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Motif DEFAULT_MSG MOTIF_LIBRARIES MOTIF_INCLUDE_DIR)


mark_as_advanced(
  MOTIF_INCLUDE_DIR
  MOTIF_LIBRARIES
)
