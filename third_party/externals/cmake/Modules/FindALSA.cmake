#.rst:
# FindALSA
# --------
#
# Find alsa
#
# Find the alsa libraries (asound)
#
# ::
#
#   This module defines the following variables:
#      ALSA_FOUND       - True if ALSA_INCLUDE_DIR & ALSA_LIBRARY are found
#      ALSA_LIBRARIES   - Set when ALSA_LIBRARY is found
#      ALSA_INCLUDE_DIRS - Set when ALSA_INCLUDE_DIR is found
#
#
#
# ::
#
#      ALSA_INCLUDE_DIR - where to find asoundlib.h, etc.
#      ALSA_LIBRARY     - the asound library
#      ALSA_VERSION_STRING - the version of alsa found (since CMake 2.8.8)

#=============================================================================
# Copyright 2009-2011 Kitware, Inc.
# Copyright 2009-2011 Philip Lowman <philip@yhbt.com>
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

find_path(ALSA_INCLUDE_DIR NAMES alsa/asoundlib.h
          DOC "The ALSA (asound) include directory"
)

find_library(ALSA_LIBRARY NAMES asound
          DOC "The ALSA (asound) library"
)

if(ALSA_INCLUDE_DIR AND EXISTS "${ALSA_INCLUDE_DIR}/alsa/version.h")
  file(STRINGS "${ALSA_INCLUDE_DIR}/alsa/version.h" alsa_version_str REGEX "^#define[\t ]+SND_LIB_VERSION_STR[\t ]+\".*\"")

  string(REGEX REPLACE "^.*SND_LIB_VERSION_STR[\t ]+\"([^\"]*)\".*$" "\\1" ALSA_VERSION_STRING "${alsa_version_str}")
  unset(alsa_version_str)
endif()

# handle the QUIETLY and REQUIRED arguments and set ALSA_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALSA
                                  REQUIRED_VARS ALSA_LIBRARY ALSA_INCLUDE_DIR
                                  VERSION_VAR ALSA_VERSION_STRING)

if(ALSA_FOUND)
  set( ALSA_LIBRARIES ${ALSA_LIBRARY} )
  set( ALSA_INCLUDE_DIRS ${ALSA_INCLUDE_DIR} )
endif()

mark_as_advanced(ALSA_INCLUDE_DIR ALSA_LIBRARY)
