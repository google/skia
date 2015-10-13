
#=============================================================================
# Copyright 2009 Kitware, Inc.
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

# This file is included in CMakeSystemSpecificInformation.cmake if
# the CodeBlocks extra generator has been selected.

find_program(CMAKE_CODEBLOCKS_EXECUTABLE NAMES codeblocks DOC "The CodeBlocks executable")

if(CMAKE_CODEBLOCKS_EXECUTABLE)
   set(CMAKE_OPEN_PROJECT_COMMAND "${CMAKE_CODEBLOCKS_EXECUTABLE} <PROJECT_FILE>" )
endif()

# Determine builtin macros and include dirs:
include(${CMAKE_CURRENT_LIST_DIR}/CMakeExtraGeneratorDetermineCompilerMacrosAndIncludeDirs.cmake)
