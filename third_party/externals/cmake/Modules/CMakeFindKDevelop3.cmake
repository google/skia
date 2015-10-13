
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
# the KDevelop3 extra generator has been selected.

find_program(CMAKE_KDEVELOP3_EXECUTABLE NAMES kdevelop DOC "The KDevelop3 executable")

if(CMAKE_KDEVELOP3_EXECUTABLE)
   set(CMAKE_OPEN_PROJECT_COMMAND "${CMAKE_KDEVELOP3_EXECUTABLE} <PROJECT_FILE>" )
endif()

