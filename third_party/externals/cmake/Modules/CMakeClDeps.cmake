
#=============================================================================
# Copyright 2012 Kitware, Inc.
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

#
# When using Ninja cl.exe is wrapped by cmcldeps to extract the included
# headers for dependency tracking.
#
# cmcldeps path is set, and cmcldeps needs to know the localized string
# in front of each include path, so it can remove it.
#

if(CMAKE_GENERATOR MATCHES "Ninja" AND CMAKE_C_COMPILER AND CMAKE_COMMAND)
  string(REPLACE "cmake.exe" "cmcldeps.exe"  CMAKE_CMCLDEPS_EXECUTABLE ${CMAKE_COMMAND})
  set(showdir ${CMAKE_BINARY_DIR}/CMakeFiles/ShowIncludes)
  file(WRITE ${showdir}/foo.h "\n")
  file(WRITE ${showdir}/main.c "#include \"foo.h\" \nint main(){}\n")
  execute_process(COMMAND ${CMAKE_C_COMPILER} /nologo /showIncludes ${showdir}/main.c
                  WORKING_DIRECTORY ${showdir} OUTPUT_VARIABLE outLine)
  string(REGEX MATCH "\n([^:]*:[^:]*:[ \t]*)" tmp "${outLine}")
  set(localizedPrefix "${CMAKE_MATCH_1}")
  set(SET_CMAKE_CMCLDEPS_EXECUTABLE   "set(CMAKE_CMCLDEPS_EXECUTABLE \"${CMAKE_CMCLDEPS_EXECUTABLE}\")")
  set(SET_CMAKE_CL_SHOWINCLUDES_PREFIX "set(CMAKE_CL_SHOWINCLUDES_PREFIX \"${localizedPrefix}\")")
endif()
