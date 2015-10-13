#.rst:
# FindUnixCommands
# ----------------
#
# Find Unix commands, including the ones from Cygwin
#
# This module looks for the Unix commands bash, cp, gzip, mv, rm, and tar
# and stores the result in the variables BASH, CP, GZIP, MV, RM, and TAR.

#=============================================================================
# Copyright 2001-2014 Kitware, Inc.
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

find_program(BASH
  bash
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)
mark_as_advanced(
  BASH
)

find_program(CP
  cp
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)
mark_as_advanced(
  CP
)

find_program(GZIP
  gzip
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)
mark_as_advanced(
  GZIP
)

find_program(MV
  mv
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)
mark_as_advanced(
  MV
)

find_program(RM
  rm
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)
mark_as_advanced(
  RM
)

find_program(TAR
  NAMES
  tar
  gtar
  PATH
  ${CYGWIN_INSTALL_PATH}/bin
  /bin
  /usr/bin
  /usr/local/bin
  /sbin
)
mark_as_advanced(
  TAR
)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(UnixCommands
  REQUIRED_VARS BASH CP GZIP MV RM TAR
)
