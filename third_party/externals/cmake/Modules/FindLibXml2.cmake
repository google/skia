#.rst:
# FindLibXml2
# -----------
#
# Try to find the LibXml2 xml processing library
#
# Once done this will define
#
# ::
#
#   LIBXML2_FOUND - System has LibXml2
#   LIBXML2_INCLUDE_DIR - The LibXml2 include directory
#   LIBXML2_LIBRARIES - The libraries needed to use LibXml2
#   LIBXML2_DEFINITIONS - Compiler switches required for using LibXml2
#   LIBXML2_XMLLINT_EXECUTABLE - The XML checking tool xmllint coming with LibXml2
#   LIBXML2_VERSION_STRING - the version of LibXml2 found (since CMake 2.8.8)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls
find_package(PkgConfig QUIET)
PKG_CHECK_MODULES(PC_LIBXML QUIET libxml-2.0)
set(LIBXML2_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

find_path(LIBXML2_INCLUDE_DIR NAMES libxml/xpath.h
   HINTS
   ${PC_LIBXML_INCLUDEDIR}
   ${PC_LIBXML_INCLUDE_DIRS}
   PATH_SUFFIXES libxml2
   )

find_library(LIBXML2_LIBRARIES NAMES xml2 libxml2
   HINTS
   ${PC_LIBXML_LIBDIR}
   ${PC_LIBXML_LIBRARY_DIRS}
   )

find_program(LIBXML2_XMLLINT_EXECUTABLE xmllint)
# for backwards compat. with KDE 4.0.x:
set(XMLLINT_EXECUTABLE "${LIBXML2_XMLLINT_EXECUTABLE}")

if(PC_LIBXML_VERSION)
    set(LIBXML2_VERSION_STRING ${PC_LIBXML_VERSION})
elseif(LIBXML2_INCLUDE_DIR AND EXISTS "${LIBXML2_INCLUDE_DIR}/libxml/xmlversion.h")
    file(STRINGS "${LIBXML2_INCLUDE_DIR}/libxml/xmlversion.h" libxml2_version_str
         REGEX "^#define[\t ]+LIBXML_DOTTED_VERSION[\t ]+\".*\"")

    string(REGEX REPLACE "^#define[\t ]+LIBXML_DOTTED_VERSION[\t ]+\"([^\"]*)\".*" "\\1"
           LIBXML2_VERSION_STRING "${libxml2_version_str}")
    unset(libxml2_version_str)
endif()

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXml2
                                  REQUIRED_VARS LIBXML2_LIBRARIES LIBXML2_INCLUDE_DIR
                                  VERSION_VAR LIBXML2_VERSION_STRING)

mark_as_advanced(LIBXML2_INCLUDE_DIR LIBXML2_LIBRARIES LIBXML2_XMLLINT_EXECUTABLE)
