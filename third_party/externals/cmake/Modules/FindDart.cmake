#.rst:
# FindDart
# --------
#
# Find DART
#
# This module looks for the dart testing software and sets DART_ROOT to
# point to where it found it.

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

find_path(DART_ROOT README.INSTALL
    HINTS
      ENV DART_ROOT
    PATHS
      ${PROJECT_SOURCE_DIR}
      /usr/share
      C:/
      "C:/Program Files"
      ${PROJECT_SOURCE_DIR}/..
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Dart\\InstallPath]
      ENV ProgramFiles
    PATH_SUFFIXES
      Dart
    DOC "If you have Dart installed, where is it located?"
    )

# handle the QUIETLY and REQUIRED arguments and set DART_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Dart DEFAULT_MSG DART_ROOT)

mark_as_advanced(DART_ROOT)
