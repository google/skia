
#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

if(UNIX)
  set(CMAKE_ASM${ASM_DIALECT}_OUTPUT_EXTENSION .o)
else()
  set(CMAKE_ASM${ASM_DIALECT}_OUTPUT_EXTENSION .obj)
endif()

set(CMAKE_INCLUDE_FLAG_ASM${ASM_DIALECT} "-I")       # -I
set(CMAKE_BASE_NAME)
get_filename_component(CMAKE_BASE_NAME "${CMAKE_ASM${ASM_DIALECT}_COMPILER}" NAME_WE)

if("${CMAKE_BASE_NAME}" STREQUAL "as")
  set(CMAKE_BASE_NAME gas)
endif()

# Load compiler-specific information.
set(_INCLUDED_FILE "")
if(CMAKE_ASM${ASM_DIALECT}_COMPILER_ID)
  include(Compiler/${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID}-ASM${ASM_DIALECT} OPTIONAL  RESULT_VARIABLE _INCLUDED_FILE)
endif()
if(NOT _INCLUDED_FILE)
  if("ASM${ASM_DIALECT}" STREQUAL "ASM")
    message(STATUS "Warning: Did not find file Compiler/${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID}-ASM${ASM_DIALECT}")
  endif()
  include(Platform/${CMAKE_BASE_NAME} OPTIONAL)
endif()

if(CMAKE_SYSTEM_PROCESSOR)
  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID}-ASM${ASM_DIALECT}-${CMAKE_SYSTEM_PROCESSOR} OPTIONAL  RESULT_VARIABLE _INCLUDED_FILE)
  if(NOT _INCLUDED_FILE)
    include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME}-${CMAKE_SYSTEM_PROCESSOR} OPTIONAL)
  endif()
endif()

include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_ASM${ASM_DIALECT}_COMPILER_ID}-ASM${ASM_DIALECT} OPTIONAL  RESULT_VARIABLE _INCLUDED_FILE)
if(NOT _INCLUDED_FILE)
  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME} OPTIONAL)
endif()

# This should be included before the _INIT variables are
# used to initialize the cache.  Since the rule variables
# have if blocks on them, users can still define them here.
# But, it should still be after the platform file so changes can
# be made to those values.

if(CMAKE_USER_MAKE_RULES_OVERRIDE)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE "${_override}")
endif()

if(CMAKE_USER_MAKE_RULES_OVERRIDE_ASM)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE_ASM} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE_ASM "${_override}")
endif()

# Set default assembler file extensions:
if(NOT CMAKE_ASM${ASM_DIALECT}_SOURCE_FILE_EXTENSIONS)
  set(CMAKE_ASM${ASM_DIALECT}_SOURCE_FILE_EXTENSIONS s;S;asm)
endif()


# Support for CMAKE_ASM${ASM_DIALECT}_FLAGS_INIT and friends:
set(CMAKE_ASM${ASM_DIALECT}_FLAGS_INIT "$ENV{ASM${ASM_DIALECT}FLAGS} ${CMAKE_ASM${ASM_DIALECT}_FLAGS_INIT}")
# avoid just having a space as the initial value for the cache
if(CMAKE_ASM${ASM_DIALECT}_FLAGS_INIT STREQUAL " ")
  set(CMAKE_ASM${ASM_DIALECT}_FLAGS_INIT)
endif()
set (CMAKE_ASM${ASM_DIALECT}_FLAGS "${CMAKE_ASM${ASM_DIALECT}_FLAGS_INIT}" CACHE STRING
     "Flags used by the assembler during all build types.")

if(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
# default build type is none
  if(NOT CMAKE_NO_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_INIT} CACHE STRING
      "Choose the type of build, options are: None, Debug Release RelWithDebInfo MinSizeRel.")
  endif()
  set (CMAKE_ASM${ASM_DIALECT}_FLAGS_DEBUG "${CMAKE_ASM${ASM_DIALECT}_FLAGS_DEBUG_INIT}" CACHE STRING
    "Flags used by the assembler during debug builds.")
  set (CMAKE_ASM${ASM_DIALECT}_FLAGS_MINSIZEREL "${CMAKE_ASM${ASM_DIALECT}_FLAGS_MINSIZEREL_INIT}" CACHE STRING
    "Flags used by the assembler during release minsize builds.")
  set (CMAKE_ASM${ASM_DIALECT}_FLAGS_RELEASE "${CMAKE_ASM${ASM_DIALECT}_FLAGS_RELEASE_INIT}" CACHE STRING
    "Flags used by the assembler during release builds.")
  set (CMAKE_ASM${ASM_DIALECT}_FLAGS_RELWITHDEBINFO "${CMAKE_ASM${ASM_DIALECT}_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
    "Flags used by the assembler during Release with Debug Info builds.")
endif()

mark_as_advanced(CMAKE_ASM${ASM_DIALECT}_FLAGS
                 CMAKE_ASM${ASM_DIALECT}_FLAGS_DEBUG
                 CMAKE_ASM${ASM_DIALECT}_FLAGS_MINSIZEREL
                 CMAKE_ASM${ASM_DIALECT}_FLAGS_RELEASE
                 CMAKE_ASM${ASM_DIALECT}_FLAGS_RELWITHDEBINFO
                )


if(NOT CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT)
  set(CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT "<CMAKE_ASM${ASM_DIALECT}_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
endif()

if(NOT CMAKE_ASM${ASM_DIALECT}_CREATE_STATIC_LIBRARY)
  set(CMAKE_ASM${ASM_DIALECT}_CREATE_STATIC_LIBRARY
      "<CMAKE_AR> cr <TARGET> <LINK_FLAGS> <OBJECTS> "
      "<CMAKE_RANLIB> <TARGET> ")
endif()

if(NOT CMAKE_ASM${ASM_DIALECT}_LINK_EXECUTABLE)
  set(CMAKE_ASM${ASM_DIALECT}_LINK_EXECUTABLE
    "<CMAKE_ASM${ASM_DIALECT}_COMPILER> <FLAGS> <CMAKE_ASM${ASM_DIALECT}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
endif()

if(NOT CMAKE_EXECUTABLE_RUNTIME_ASM${ASM_DIALECT}_FLAG)
  set(CMAKE_EXECUTABLE_RUNTIME_ASM${ASM_DIALECT}_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_ASM${ASM_DIALECT}_FLAG})
endif()

if(NOT CMAKE_EXECUTABLE_RUNTIME_ASM${ASM_DIALECT}_FLAG_SEP)
  set(CMAKE_EXECUTABLE_RUNTIME_ASM${ASM_DIALECT}_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_ASM${ASM_DIALECT}_FLAG_SEP})
endif()

if(NOT CMAKE_EXECUTABLE_RPATH_LINK_ASM${ASM_DIALECT}_FLAG)
  set(CMAKE_EXECUTABLE_RPATH_LINK_ASM${ASM_DIALECT}_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_ASM${ASM_DIALECT}_FLAG})
endif()

# to be done
if(NOT CMAKE_ASM${ASM_DIALECT}_CREATE_SHARED_LIBRARY)
  set(CMAKE_ASM${ASM_DIALECT}_CREATE_SHARED_LIBRARY)
endif()

if(NOT CMAKE_ASM${ASM_DIALECT}_CREATE_SHARED_MODULE)
  set(CMAKE_ASM${ASM_DIALECT}_CREATE_SHARED_MODULE)
endif()


set(CMAKE_ASM${ASM_DIALECT}_INFOMATION_LOADED 1)

