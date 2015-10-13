#.rst:
# FindMFC
# -------
#
# Find MFC on Windows
#
# Find the native MFC - i.e.  decide if an application can link to the
# MFC libraries.
#
# ::
#
#   MFC_FOUND - Was MFC support found
#
# You don't need to include anything or link anything to use it.

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

# Assume no MFC support
set(MFC_FOUND "NO")

# Only attempt the try_compile call if it has a chance to succeed:
set(MFC_ATTEMPT_TRY_COMPILE 0)
if(WIN32 AND NOT UNIX AND NOT BORLAND AND NOT MINGW)
  set(MFC_ATTEMPT_TRY_COMPILE 1)
endif()

if(MFC_ATTEMPT_TRY_COMPILE)
  if(NOT DEFINED MFC_HAVE_MFC)
    set(CHECK_INCLUDE_FILE_VAR "afxwin.h")
    configure_file(${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.cxx)
    message(STATUS "Looking for MFC")
    try_compile(MFC_HAVE_MFC
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckIncludeFile.cxx
      CMAKE_FLAGS
      -DCMAKE_MFC_FLAG:STRING=2
      -DCOMPILE_DEFINITIONS:STRING=-D_AFXDLL
      OUTPUT_VARIABLE OUTPUT)
    if(MFC_HAVE_MFC)
      message(STATUS "Looking for MFC - found")
      set(MFC_HAVE_MFC 1 CACHE INTERNAL "Have MFC?")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if MFC exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    else()
      message(STATUS "Looking for MFC - not found")
      set(MFC_HAVE_MFC 0 CACHE INTERNAL "Have MFC?")
      file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if MFC exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    endif()
  endif()

  if(MFC_HAVE_MFC)
    set(MFC_FOUND "YES")
  endif()
endif()
