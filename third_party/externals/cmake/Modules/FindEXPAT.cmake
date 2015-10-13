#.rst:
# FindEXPAT
# ---------
#
# Find expat
#
# Find the native EXPAT headers and libraries.
#
# ::
#
#   EXPAT_INCLUDE_DIRS - where to find expat.h, etc.
#   EXPAT_LIBRARIES    - List of libraries when using expat.
#   EXPAT_FOUND        - True if expat found.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

# Look for the header file.
find_path(EXPAT_INCLUDE_DIR NAMES expat.h)

# Look for the library.
find_library(EXPAT_LIBRARY NAMES expat libexpat)

if (EXPAT_INCLUDE_DIR AND EXISTS "${EXPAT_INCLUDE_DIR}/expat.h")
    file(STRINGS "${EXPAT_INCLUDE_DIR}/expat.h" expat_version_str
         REGEX "^#[\t ]*define[\t ]+XML_(MAJOR|MINOR|MICRO)_VERSION[\t ]+[0-9]+$")

    unset(EXPAT_VERSION_STRING)
    foreach(VPART MAJOR MINOR MICRO)
        foreach(VLINE ${expat_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+XML_${VPART}_VERSION[\t ]+([0-9]+)$")
                set(EXPAT_VERSION_PART "${CMAKE_MATCH_1}")
                if(EXPAT_VERSION_STRING)
                    set(EXPAT_VERSION_STRING "${EXPAT_VERSION_STRING}.${EXPAT_VERSION_PART}")
                else()
                    set(EXPAT_VERSION_STRING "${EXPAT_VERSION_PART}")
                endif()
            endif()
        endforeach()
    endforeach()
endif ()

# handle the QUIETLY and REQUIRED arguments and set EXPAT_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EXPAT
                                  REQUIRED_VARS EXPAT_LIBRARY EXPAT_INCLUDE_DIR
                                  VERSION_VAR EXPAT_VERSION_STRING)

# Copy the results to the output variables.
if(EXPAT_FOUND)
  set(EXPAT_LIBRARIES ${EXPAT_LIBRARY})
  set(EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIR})
endif()

mark_as_advanced(EXPAT_INCLUDE_DIR EXPAT_LIBRARY)
