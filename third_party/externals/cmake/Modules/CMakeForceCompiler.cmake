#.rst:
# CMakeForceCompiler
# ------------------
#
#
#
# This module defines macros intended for use by cross-compiling
# toolchain files when CMake is not able to automatically detect the
# compiler identification.
#
# Macro CMAKE_FORCE_C_COMPILER has the following signature:
#
# ::
#
#    CMAKE_FORCE_C_COMPILER(<compiler> <compiler-id>)
#
# It sets CMAKE_C_COMPILER to the given compiler and the cmake internal
# variable CMAKE_C_COMPILER_ID to the given compiler-id.  It also
# bypasses the check for working compiler and basic compiler information
# tests.
#
# Macro CMAKE_FORCE_CXX_COMPILER has the following signature:
#
# ::
#
#    CMAKE_FORCE_CXX_COMPILER(<compiler> <compiler-id>)
#
# It sets CMAKE_CXX_COMPILER to the given compiler and the cmake
# internal variable CMAKE_CXX_COMPILER_ID to the given compiler-id.  It
# also bypasses the check for working compiler and basic compiler
# information tests.
#
# Macro CMAKE_FORCE_Fortran_COMPILER has the following signature:
#
# ::
#
#    CMAKE_FORCE_Fortran_COMPILER(<compiler> <compiler-id>)
#
# It sets CMAKE_Fortran_COMPILER to the given compiler and the cmake
# internal variable CMAKE_Fortran_COMPILER_ID to the given compiler-id.
# It also bypasses the check for working compiler and basic compiler
# information tests.
#
# So a simple toolchain file could look like this:
#
# ::
#
#    include (CMakeForceCompiler)
#    set(CMAKE_SYSTEM_NAME Generic)
#    CMAKE_FORCE_C_COMPILER   (chc12 MetrowerksHicross)
#    CMAKE_FORCE_CXX_COMPILER (chc12 MetrowerksHicross)

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

macro(CMAKE_FORCE_C_COMPILER compiler id)
  set(CMAKE_C_COMPILER "${compiler}")
  set(CMAKE_C_COMPILER_ID_RUN TRUE)
  set(CMAKE_C_COMPILER_ID ${id})
  set(CMAKE_C_COMPILER_FORCED TRUE)

  # Set old compiler id variables.
  if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCC 1)
  endif()
endmacro()

macro(CMAKE_FORCE_CXX_COMPILER compiler id)
  set(CMAKE_CXX_COMPILER "${compiler}")
  set(CMAKE_CXX_COMPILER_ID_RUN TRUE)
  set(CMAKE_CXX_COMPILER_ID ${id})
  set(CMAKE_CXX_COMPILER_FORCED TRUE)

  # Set old compiler id variables.
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCXX 1)
  endif()
endmacro()

macro(CMAKE_FORCE_Fortran_COMPILER compiler id)
  set(CMAKE_Fortran_COMPILER "${compiler}")
  set(CMAKE_Fortran_COMPILER_ID_RUN TRUE)
  set(CMAKE_Fortran_COMPILER_ID ${id})
  set(CMAKE_Fortran_COMPILER_FORCED TRUE)

  # Set old compiler id variables.
  if(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUG77 1)
  endif()
endmacro()
