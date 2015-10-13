#.rst:
# FindBZip2
# ---------
#
# Try to find BZip2
#
# Once done this will define
#
# ::
#
#   BZIP2_FOUND - system has BZip2
#   BZIP2_INCLUDE_DIR - the BZip2 include directory
#   BZIP2_LIBRARIES - Link these to use BZip2
#   BZIP2_NEED_PREFIX - this is set if the functions are prefixed with BZ2_
#   BZIP2_VERSION_STRING - the version of BZip2 found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2012 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
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

set(_BZIP2_PATHS PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\Bzip2;InstallPath]"
  )

find_path(BZIP2_INCLUDE_DIR bzlib.h ${_BZIP2_PATHS} PATH_SUFFIXES include)

if (NOT BZIP2_LIBRARIES)
    find_library(BZIP2_LIBRARY_RELEASE NAMES bz2 bzip2 ${_BZIP2_PATHS} PATH_SUFFIXES lib)
    find_library(BZIP2_LIBRARY_DEBUG NAMES bzip2d ${_BZIP2_PATHS} PATH_SUFFIXES lib)

    include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
    SELECT_LIBRARY_CONFIGURATIONS(BZIP2)
endif ()

if (BZIP2_INCLUDE_DIR AND EXISTS "${BZIP2_INCLUDE_DIR}/bzlib.h")
    file(STRINGS "${BZIP2_INCLUDE_DIR}/bzlib.h" BZLIB_H REGEX "bzip2/libbzip2 version [0-9]+\\.[^ ]+ of [0-9]+ ")
    string(REGEX REPLACE ".* bzip2/libbzip2 version ([0-9]+\\.[^ ]+) of [0-9]+ .*" "\\1" BZIP2_VERSION_STRING "${BZLIB_H}")
endif ()

# handle the QUIETLY and REQUIRED arguments and set BZip2_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BZip2
                                  REQUIRED_VARS BZIP2_LIBRARIES BZIP2_INCLUDE_DIR
                                  VERSION_VAR BZIP2_VERSION_STRING)

if (BZIP2_FOUND)
   include(${CMAKE_CURRENT_LIST_DIR}/CheckLibraryExists.cmake)
   include(${CMAKE_CURRENT_LIST_DIR}/CMakePushCheckState.cmake)
   cmake_push_check_state()
   set(CMAKE_REQUIRED_QUIET ${BZip2_FIND_QUIETLY})
   CHECK_LIBRARY_EXISTS("${BZIP2_LIBRARIES}" BZ2_bzCompressInit "" BZIP2_NEED_PREFIX)
   cmake_pop_check_state()
endif ()

mark_as_advanced(BZIP2_INCLUDE_DIR)
