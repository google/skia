#.rst:
# CMakePushCheckState
# -------------------
#
#
#
# This module defines three macros: CMAKE_PUSH_CHECK_STATE()
# CMAKE_POP_CHECK_STATE() and CMAKE_RESET_CHECK_STATE() These macros can
# be used to save, restore and reset (i.e., clear contents) the state of
# the variables CMAKE_REQUIRED_FLAGS, CMAKE_REQUIRED_DEFINITIONS,
# CMAKE_REQUIRED_LIBRARIES and CMAKE_REQUIRED_INCLUDES used by the
# various Check-files coming with CMake, like e.g.
# check_function_exists() etc.  The variable contents are pushed on a
# stack, pushing multiple times is supported.  This is useful e.g.  when
# executing such tests in a Find-module, where they have to be set, but
# after the Find-module has been executed they should have the same
# value as they had before.
#
# CMAKE_PUSH_CHECK_STATE() macro receives optional argument RESET.
# Whether it's specified, CMAKE_PUSH_CHECK_STATE() will set all
# CMAKE_REQUIRED_* variables to empty values, same as
# CMAKE_RESET_CHECK_STATE() call will do.
#
# Usage:
#
# ::
#
#    cmake_push_check_state(RESET)
#    set(CMAKE_REQUIRED_DEFINITIONS -DSOME_MORE_DEF)
#    check_function_exists(...)
#    cmake_reset_check_state()
#    set(CMAKE_REQUIRED_DEFINITIONS -DANOTHER_DEF)
#    check_function_exists(...)
#    cmake_pop_check_state()

#=============================================================================
# Copyright 2006-2011 Alexander Neundorf, <neundorf@kde.org>
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


macro(CMAKE_RESET_CHECK_STATE)

   set(CMAKE_REQUIRED_INCLUDES)
   set(CMAKE_REQUIRED_DEFINITIONS)
   set(CMAKE_REQUIRED_LIBRARIES)
   set(CMAKE_REQUIRED_FLAGS)
   set(CMAKE_REQUIRED_QUIET)

endmacro()

macro(CMAKE_PUSH_CHECK_STATE)

   if(NOT DEFINED _CMAKE_PUSH_CHECK_STATE_COUNTER)
      set(_CMAKE_PUSH_CHECK_STATE_COUNTER 0)
   endif()

   math(EXPR _CMAKE_PUSH_CHECK_STATE_COUNTER "${_CMAKE_PUSH_CHECK_STATE_COUNTER}+1")

   set(_CMAKE_REQUIRED_INCLUDES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}    ${CMAKE_REQUIRED_INCLUDES})
   set(_CMAKE_REQUIRED_DEFINITIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER} ${CMAKE_REQUIRED_DEFINITIONS})
   set(_CMAKE_REQUIRED_LIBRARIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}   ${CMAKE_REQUIRED_LIBRARIES})
   set(_CMAKE_REQUIRED_FLAGS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}       ${CMAKE_REQUIRED_FLAGS})
   set(_CMAKE_REQUIRED_QUIET_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}       ${CMAKE_REQUIRED_QUIET})

   if (ARGC GREATER 0 AND ARGV0 STREQUAL "RESET")
      cmake_reset_check_state()
   endif()

endmacro()

macro(CMAKE_POP_CHECK_STATE)

# don't pop more than we pushed
   if("${_CMAKE_PUSH_CHECK_STATE_COUNTER}" GREATER "0")

      set(CMAKE_REQUIRED_INCLUDES    ${_CMAKE_REQUIRED_INCLUDES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      set(CMAKE_REQUIRED_LIBRARIES   ${_CMAKE_REQUIRED_LIBRARIES_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      set(CMAKE_REQUIRED_FLAGS       ${_CMAKE_REQUIRED_FLAGS_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})
      set(CMAKE_REQUIRED_QUIET       ${_CMAKE_REQUIRED_QUIET_SAVE_${_CMAKE_PUSH_CHECK_STATE_COUNTER}})

      math(EXPR _CMAKE_PUSH_CHECK_STATE_COUNTER "${_CMAKE_PUSH_CHECK_STATE_COUNTER}-1")
   endif()

endmacro()
