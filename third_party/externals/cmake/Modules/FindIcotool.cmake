#.rst:
# FindIcotool
# -----------
#
# Find icotool
#
# This module looks for icotool.  This module defines the following
# values:
#
# ::
#
#   ICOTOOL_EXECUTABLE: the full path to the icotool tool.
#   ICOTOOL_FOUND: True if icotool has been found.
#   ICOTOOL_VERSION_STRING: the version of icotool found.

#=============================================================================
# Copyright 2012 Aleksey Avdeev <solo@altlinux.ru>
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

find_program(ICOTOOL_EXECUTABLE
  icotool
)

if(ICOTOOL_EXECUTABLE)
  execute_process(
    COMMAND ${ICOTOOL_EXECUTABLE} --version
    OUTPUT_VARIABLE _icotool_version
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if("${_icotool_version}" MATCHES "^icotool \\([^\\)]*\\) ([0-9\\.]+[^ \n]*)")
    set( ICOTOOL_VERSION_STRING
      "${CMAKE_MATCH_1}"
    )
  else()
    set( ICOTOOL_VERSION_STRING
      ""
    )
  endif()
  unset(_icotool_version)
endif()

# handle the QUIETLY and REQUIRED arguments and set ICOTOOL_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  Icotool
  REQUIRED_VARS ICOTOOL_EXECUTABLE
  VERSION_VAR ICOTOOL_VERSION_STRING
)

mark_as_advanced(
  ICOTOOL_EXECUTABLE
)
