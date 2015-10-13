#.rst:
# FindGnuplot
# -----------
#
# this module looks for gnuplot
#
#
#
# Once done this will define
#
# ::
#
#   GNUPLOT_FOUND - system has Gnuplot
#   GNUPLOT_EXECUTABLE - the Gnuplot executable
#   GNUPLOT_VERSION_STRING - the version of Gnuplot found (since CMake 2.8.8)
#
#
#
# GNUPLOT_VERSION_STRING will not work for old versions like 3.7.1.

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

include(${CMAKE_CURRENT_LIST_DIR}/FindCygwin.cmake)

find_program(GNUPLOT_EXECUTABLE
  NAMES
  gnuplot
  pgnuplot
  wgnupl32
  PATHS
  ${CYGWIN_INSTALL_PATH}/bin
)

if (GNUPLOT_EXECUTABLE)
    execute_process(COMMAND "${GNUPLOT_EXECUTABLE}" --version
                  OUTPUT_VARIABLE GNUPLOT_OUTPUT_VARIABLE
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX REPLACE "^gnuplot ([0-9\\.]+)( patchlevel )?" "\\1." GNUPLOT_VERSION_STRING "${GNUPLOT_OUTPUT_VARIABLE}")
    string(REGEX REPLACE "\\.$" "" GNUPLOT_VERSION_STRING "${GNUPLOT_VERSION_STRING}")
    unset(GNUPLOT_OUTPUT_VARIABLE)
endif()

# for compatibility
set(GNUPLOT ${GNUPLOT_EXECUTABLE})

# handle the QUIETLY and REQUIRED arguments and set GNUPLOT_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gnuplot
                                  REQUIRED_VARS GNUPLOT_EXECUTABLE
                                  VERSION_VAR GNUPLOT_VERSION_STRING)

mark_as_advanced( GNUPLOT_EXECUTABLE )

