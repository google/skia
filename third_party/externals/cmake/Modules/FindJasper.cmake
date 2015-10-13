#.rst:
# FindJasper
# ----------
#
# Try to find the Jasper JPEG2000 library
#
# Once done this will define
#
# ::
#
#   JASPER_FOUND - system has Jasper
#   JASPER_INCLUDE_DIR - the Jasper include directory
#   JASPER_LIBRARIES - the libraries needed to use Jasper
#   JASPER_VERSION_STRING - the version of Jasper found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

find_path(JASPER_INCLUDE_DIR jasper/jasper.h)

if (NOT JASPER_LIBRARIES)
    find_package(JPEG)

    find_library(JASPER_LIBRARY_RELEASE NAMES jasper libjasper)
    find_library(JASPER_LIBRARY_DEBUG NAMES jasperd)

    include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
    SELECT_LIBRARY_CONFIGURATIONS(JASPER)
endif ()

if (JASPER_INCLUDE_DIR AND EXISTS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h")
    file(STRINGS "${JASPER_INCLUDE_DIR}/jasper/jas_config.h" jasper_version_str REGEX "^#define[\t ]+JAS_VERSION[\t ]+\".*\".*")

    string(REGEX REPLACE "^#define[\t ]+JAS_VERSION[\t ]+\"([^\"]+)\".*" "\\1" JASPER_VERSION_STRING "${jasper_version_str}")
endif ()

# handle the QUIETLY and REQUIRED arguments and set JASPER_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Jasper
                                  REQUIRED_VARS JASPER_LIBRARIES JASPER_INCLUDE_DIR JPEG_LIBRARIES
                                  VERSION_VAR JASPER_VERSION_STRING)

if (JASPER_FOUND)
   set(JASPER_LIBRARIES ${JASPER_LIBRARIES} ${JPEG_LIBRARIES} )
endif ()

mark_as_advanced(JASPER_INCLUDE_DIR)
