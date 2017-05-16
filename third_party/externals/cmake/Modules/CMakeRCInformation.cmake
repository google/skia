
#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
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


# This file sets the basic flags for the Windows Resource Compiler.
# It also loads the available platform file for the system-compiler
# if it exists.

# make sure we don't use CMAKE_BASE_NAME from somewhere else
set(CMAKE_BASE_NAME)
if(CMAKE_RC_COMPILER MATCHES "windres[^/]*$")
 set(CMAKE_BASE_NAME "windres")
else()
 get_filename_component(CMAKE_BASE_NAME ${CMAKE_RC_COMPILER} NAME_WE)
endif()
set(CMAKE_SYSTEM_AND_RC_COMPILER_INFO_FILE
  ${CMAKE_ROOT}/Modules/Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}.cmake)
include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME} OPTIONAL)



set (CMAKE_RC_FLAGS "$ENV{RCFLAGS} ${CMAKE_RC_FLAGS_INIT}" CACHE STRING
     "Flags for Windows Resource Compiler.")

# These are the only types of flags that should be passed to the rc
# command, if COMPILE_FLAGS is used on a target this will be used
# to filter out any other flags
set(CMAKE_RC_FLAG_REGEX "^[-/](D|I)")

# now define the following rule variables
# CMAKE_RC_COMPILE_OBJECT
set(CMAKE_INCLUDE_FLAG_RC "-I")
# compile a Resource file into an object file
if(NOT CMAKE_RC_COMPILE_OBJECT)
  set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> /fo<OBJECT> <SOURCE>")
endif()

mark_as_advanced(
CMAKE_RC_FLAGS
)
# set this variable so we can avoid loading this more than once.
set(CMAKE_RC_INFORMATION_LOADED 1)
