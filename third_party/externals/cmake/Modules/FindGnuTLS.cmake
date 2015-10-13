#.rst:
# FindGnuTLS
# ----------
#
# Try to find the GNU Transport Layer Security library (gnutls)
#
#
#
# Once done this will define
#
# ::
#
#   GNUTLS_FOUND - System has gnutls
#   GNUTLS_INCLUDE_DIR - The gnutls include directory
#   GNUTLS_LIBRARIES - The libraries needed to use gnutls
#   GNUTLS_DEFINITIONS - Compiler switches required for using gnutls

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
# Copyright 2009 Brad Hards <bradh@kde.org>
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
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

# Note that this doesn't try to find the gnutls-extra package.


if (GNUTLS_INCLUDE_DIR AND GNUTLS_LIBRARY)
   # in cache already
   set(gnutls_FIND_QUIETLY TRUE)
endif ()

if (NOT WIN32)
   # try using pkg-config to get the directories and then use these values
   # in the find_path() and find_library() calls
   # also fills in GNUTLS_DEFINITIONS, although that isn't normally useful
   find_package(PkgConfig QUIET)
   PKG_CHECK_MODULES(PC_GNUTLS QUIET gnutls)
   set(GNUTLS_DEFINITIONS ${PC_GNUTLS_CFLAGS_OTHER})
   set(GNUTLS_VERSION_STRING ${PC_GNUTLS_VERSION})
endif ()

find_path(GNUTLS_INCLUDE_DIR gnutls/gnutls.h
   HINTS
   ${PC_GNUTLS_INCLUDEDIR}
   ${PC_GNUTLS_INCLUDE_DIRS}
   )

find_library(GNUTLS_LIBRARY NAMES gnutls libgnutls
   HINTS
   ${PC_GNUTLS_LIBDIR}
   ${PC_GNUTLS_LIBRARY_DIRS}
   )

mark_as_advanced(GNUTLS_INCLUDE_DIR GNUTLS_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set GNUTLS_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GnuTLS
                                  REQUIRED_VARS GNUTLS_LIBRARY GNUTLS_INCLUDE_DIR
                                  VERSION_VAR GNUTLS_VERSION_STRING)

if(GNUTLS_FOUND)
    set(GNUTLS_LIBRARIES    ${GNUTLS_LIBRARY})
    set(GNUTLS_INCLUDE_DIRS ${GNUTLS_INCLUDE_DIR})
endif()

