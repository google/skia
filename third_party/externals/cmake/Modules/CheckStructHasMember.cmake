#.rst:
# CheckStructHasMember
# --------------------
#
# Check if the given struct or class has the specified member variable
#
# ::
#
#  CHECK_STRUCT_HAS_MEMBER(<struct> <member> <header> <variable>
#                          [LANGUAGE <language>])
#
# ::
#
#   <struct> - the name of the struct or class you are interested in
#   <member> - the member which existence you want to check
#   <header> - the header(s) where the prototype should be declared
#   <variable> - variable to store the result
#   <language> - the compiler to use (C or CXX)
#
#
#
# The following variables may be set before calling this macro to modify
# the way the check is run:
#
# ::
#
#   CMAKE_REQUIRED_FLAGS = string of compile command line flags
#   CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#   CMAKE_REQUIRED_INCLUDES = list of include directories
#   CMAKE_REQUIRED_LIBRARIES = list of libraries to link
#   CMAKE_REQUIRED_QUIET = execute quietly without messages
#
#
#
# Example: CHECK_STRUCT_HAS_MEMBER("struct timeval" tv_sec sys/select.h
# HAVE_TIMEVAL_TV_SEC LANGUAGE C)

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

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

macro (CHECK_STRUCT_HAS_MEMBER _STRUCT _MEMBER _HEADER _RESULT)
   set(_INCLUDE_FILES)
   foreach (it ${_HEADER})
      set(_INCLUDE_FILES "${_INCLUDE_FILES}#include <${it}>\n")
   endforeach ()

   if("x${ARGN}" STREQUAL "x")
      set(_lang C)
   elseif("x${ARGN}" MATCHES "^xLANGUAGE;([a-zA-Z]+)$")
      set(_lang "${CMAKE_MATCH_1}")
   else()
      message(FATAL_ERROR "Unknown arguments:\n  ${ARGN}\n")
   endif()

   set(_CHECK_STRUCT_MEMBER_SOURCE_CODE "
${_INCLUDE_FILES}
int main()
{
   (void)sizeof(((${_STRUCT} *)0)->${_MEMBER});
   return 0;
}
")

   if("${_lang}" STREQUAL "C")
      CHECK_C_SOURCE_COMPILES("${_CHECK_STRUCT_MEMBER_SOURCE_CODE}" ${_RESULT})
   elseif("${_lang}" STREQUAL "CXX")
      CHECK_CXX_SOURCE_COMPILES("${_CHECK_STRUCT_MEMBER_SOURCE_CODE}" ${_RESULT})
   else()
      message(FATAL_ERROR "Unknown language:\n  ${_lang}\nSupported languages: C, CXX.\n")
   endif()
endmacro ()
