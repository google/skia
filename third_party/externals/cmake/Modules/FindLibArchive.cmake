#.rst:
# FindLibArchive
# --------------
#
# Find libarchive library and headers
#
# The module defines the following variables:
#
# ::
#
#   LibArchive_FOUND        - true if libarchive was found
#   LibArchive_INCLUDE_DIRS - include search path
#   LibArchive_LIBRARIES    - libraries to link
#   LibArchive_VERSION      - libarchive 3-component version number

#=============================================================================
# Copyright 2010 Kitware, Inc.
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

find_path(LibArchive_INCLUDE_DIR
  NAMES archive.h
  PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\LibArchive;InstallPath]/include"
  )

find_library(LibArchive_LIBRARY
  NAMES archive libarchive
  PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\LibArchive;InstallPath]/lib"
  )

mark_as_advanced(LibArchive_INCLUDE_DIR LibArchive_LIBRARY)

# Extract the version number from the header.
if(LibArchive_INCLUDE_DIR AND EXISTS "${LibArchive_INCLUDE_DIR}/archive.h")
  # The version string appears in one of two known formats in the header:
  #  #define ARCHIVE_LIBRARY_VERSION "libarchive 2.4.12"
  #  #define ARCHIVE_VERSION_STRING "libarchive 2.8.4"
  # Match either format.
  set(_LibArchive_VERSION_REGEX "^#define[ \t]+ARCHIVE[_A-Z]+VERSION[_A-Z]*[ \t]+\"libarchive +([0-9]+)\\.([0-9]+)\\.([0-9]+)[^\"]*\".*$")
  file(STRINGS "${LibArchive_INCLUDE_DIR}/archive.h" _LibArchive_VERSION_STRING LIMIT_COUNT 1 REGEX "${_LibArchive_VERSION_REGEX}")
  if(_LibArchive_VERSION_STRING)
    string(REGEX REPLACE "${_LibArchive_VERSION_REGEX}" "\\1.\\2.\\3" LibArchive_VERSION "${_LibArchive_VERSION_STRING}")
  endif()
  unset(_LibArchive_VERSION_REGEX)
  unset(_LibArchive_VERSION_STRING)
endif()

# Handle the QUIETLY and REQUIRED arguments and set LIBARCHIVE_FOUND
# to TRUE if all listed variables are TRUE.
# (Use ${CMAKE_ROOT}/Modules instead of ${CMAKE_CURRENT_LIST_DIR} because CMake
#  itself includes this FindLibArchive when built with an older CMake that does
#  not provide it.  The older CMake also does not have CMAKE_CURRENT_LIST_DIR.)
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(LibArchive
                                  REQUIRED_VARS LibArchive_LIBRARY LibArchive_INCLUDE_DIR
                                  VERSION_VAR LibArchive_VERSION
  )
set(LibArchive_FOUND ${LIBARCHIVE_FOUND})
unset(LIBARCHIVE_FOUND)

if(LibArchive_FOUND)
  set(LibArchive_INCLUDE_DIRS ${LibArchive_INCLUDE_DIR})
  set(LibArchive_LIBRARIES    ${LibArchive_LIBRARY})
endif()
