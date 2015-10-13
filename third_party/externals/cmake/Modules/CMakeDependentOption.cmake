#.rst:
# CMakeDependentOption
# --------------------
#
# Macro to provide an option dependent on other options.
#
# This macro presents an option to the user only if a set of other
# conditions are true.  When the option is not presented a default value
# is used, but any value set by the user is preserved for when the
# option is presented again.  Example invocation:
#
# ::
#
#   CMAKE_DEPENDENT_OPTION(USE_FOO "Use Foo" ON
#                          "USE_BAR;NOT USE_ZOT" OFF)
#
# If USE_BAR is true and USE_ZOT is false, this provides an option
# called USE_FOO that defaults to ON.  Otherwise, it sets USE_FOO to
# OFF.  If the status of USE_BAR or USE_ZOT ever changes, any value for
# the USE_FOO option is saved so that when the option is re-enabled it
# retains its old value.

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

macro(CMAKE_DEPENDENT_OPTION option doc default depends force)
  if(${option}_ISSET MATCHES "^${option}_ISSET$")
    set(${option}_AVAILABLE 1)
    foreach(d ${depends})
      string(REGEX REPLACE " +" ";" CMAKE_DEPENDENT_OPTION_DEP "${d}")
      if(${CMAKE_DEPENDENT_OPTION_DEP})
      else()
        set(${option}_AVAILABLE 0)
      endif()
    endforeach()
    if(${option}_AVAILABLE)
      option(${option} "${doc}" "${default}")
      set(${option} "${${option}}" CACHE BOOL "${doc}" FORCE)
    else()
      if(${option} MATCHES "^${option}$")
      else()
        set(${option} "${${option}}" CACHE INTERNAL "${doc}")
      endif()
      set(${option} ${force})
    endif()
  else()
    set(${option} "${${option}_ISSET}")
  endif()
endmacro()
