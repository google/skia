#.rst:
# FindWget
# --------
#
# Find wget
#
# This module looks for wget.  This module defines the following values:
#
# ::
#
#   WGET_EXECUTABLE: the full path to the wget tool.
#   WGET_FOUND: True if wget has been found.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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

include(${CMAKE_CURRENT_LIST_DIR}/FindCygwin.cmake)

find_program(WGET_EXECUTABLE
  wget
  ${CYGWIN_INSTALL_PATH}/bin
)

# handle the QUIETLY and REQUIRED arguments and set WGET_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Wget DEFAULT_MSG WGET_EXECUTABLE)

mark_as_advanced( WGET_EXECUTABLE )

# WGET option is deprecated.
# use WGET_EXECUTABLE instead.
set (WGET ${WGET_EXECUTABLE} )
