#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

#-----------------------------------------------------------------------------
# CMake uses KWStyle for checking the coding style

# Search for a built-from-source KWStyle under Dashboards/Support on a typical
# dashboard machines:
#
set(home "$ENV{HOME}")
if(NOT home)
  string(REPLACE "\\" "/" home "$ENV{USERPROFILE}")
endif()

find_program(KWSTYLE_EXECUTABLE
  NAMES KWStyle
  PATHS
  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Kitware Inc.\\KWStyle 1.0.0]/bin"
  "${home}/Dashboards/Support/KWStyle/bin"
  )
mark_as_advanced(KWSTYLE_EXECUTABLE)

set(CMAKE_USE_KWSTYLE_DEFAULT OFF)
if(KWSTYLE_EXECUTABLE)
  set(CMAKE_USE_KWSTYLE_DEFAULT ON)
endif()

option(CMAKE_USE_KWSTYLE
  "Add StyleCheck target and KWStyle test: run KWStyle to check for coding standard violations."
  ${CMAKE_USE_KWSTYLE_DEFAULT})
mark_as_advanced(CMAKE_USE_KWSTYLE)

if(CMAKE_USE_KWSTYLE)
  option(KWSTYLE_USE_VIM_FORMAT "Set KWStyle to generate errors with a VIM-compatible format." OFF)
  option(KWSTYLE_USE_MSVC_FORMAT "Set KWStyle to generate errors with a VisualStudio-compatible format." OFF)
  mark_as_advanced(KWSTYLE_USE_VIM_FORMAT)
  mark_as_advanced(KWSTYLE_USE_MSVC_FORMAT)

  if(KWSTYLE_USE_VIM_FORMAT)
    set(KWSTYLE_ARGUMENTS -vim ${KWSTYLE_ARGUMENTS})
  endif()

  if(KWSTYLE_USE_MSVC_FORMAT)
    set(KWSTYLE_ARGUMENTS -msvc ${KWSTYLE_ARGUMENTS})
  endif()

  configure_file(${CMake_SOURCE_DIR}/Utilities/KWStyle/CMake.kws.xml.in
               ${CMake_BINARY_DIR}/CMake.kws.xml)
  configure_file(${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeMoreChecks.kws.xml.in
               ${CMake_BINARY_DIR}/CMakeMoreChecks.kws.xml)

  configure_file(${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeFiles.txt.in
                 ${CMake_BINARY_DIR}/CMakeKWSFiles.txt)

  add_custom_command(
    OUTPUT ${CMake_BINARY_DIR}/KWStyleReport.txt
    COMMAND ${KWSTYLE_EXECUTABLE}
    ARGS    -xml ${CMake_BINARY_DIR}/CMake.kws.xml -o ${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeOverwrite.txt -v ${KWSTYLE_ARGUMENTS} -D ${CMake_BINARY_DIR}/CMakeKWSFiles.txt
    COMMENT "Coding Style Checker"
    )

  add_custom_target(MoreStyleChecks
    COMMAND ${KWSTYLE_EXECUTABLE}
    -xml ${CMake_BINARY_DIR}/CMakeMoreChecks.kws.xml -html ${CMake_BINARY_DIR}/html -o ${CMake_SOURCE_DIR}/Utilities/KWStyle/CMakeOverwrite.txt -v ${KWSTYLE_ARGUMENTS} -D ${CMake_BINARY_DIR}/CMakeKWSFiles.txt
    COMMENT "Coding Style Checker, more checks enabled"
    )

  add_custom_target(StyleCheck DEPENDS ${CMake_BINARY_DIR}/KWStyleReport.txt)
endif()
