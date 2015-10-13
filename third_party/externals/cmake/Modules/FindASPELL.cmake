#.rst:
# FindASPELL
# ----------
#
# Try to find ASPELL
#
# Once done this will define
#
# ::
#
#   ASPELL_FOUND - system has ASPELL
#   ASPELL_EXECUTABLE - the ASPELL executable
#   ASPELL_INCLUDE_DIR - the ASPELL include directory
#   ASPELL_LIBRARIES - The libraries needed to use ASPELL
#   ASPELL_DEFINITIONS - Compiler switches required for using ASPELL

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

find_path(ASPELL_INCLUDE_DIR aspell.h )

find_program(ASPELL_EXECUTABLE
  NAMES aspell
)

find_library(ASPELL_LIBRARIES NAMES aspell aspell-15 libaspell-15 libaspell)

# handle the QUIETLY and REQUIRED arguments and set ASPELL_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ASPELL DEFAULT_MSG ASPELL_LIBRARIES ASPELL_INCLUDE_DIR ASPELL_EXECUTABLE)

mark_as_advanced(ASPELL_INCLUDE_DIR ASPELL_LIBRARIES ASPELL_EXECUTABLE)
