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
# set some special flags for different compilers
#
if(CMAKE_GENERATOR MATCHES "Visual Studio 7")
  set(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
endif()

if(WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "Intel")
  set(_INTEL_WINDOWS 1)
endif()

# Disable deprecation warnings for standard C functions.
# really only needed for newer versions of VS, but should
# not hurt other versions, and this will work into the
# future
if(MSVC OR _INTEL_WINDOWS)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
else()
endif()

#silence duplicate symbol warnings on AIX
if(CMAKE_SYSTEM_NAME MATCHES "AIX")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -bhalt:5 ")
  endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES "IRIX")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-woff84 -no_auto_include")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-woff15")
  endif()
endif()

if(CMAKE_SYSTEM MATCHES "OSF1-V")
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local -no_implicit_include ")
  endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES "HP-UX" AND CMAKE_CXX_COMPILER_ID MATCHES "HP")
  # HP aCC since version 3.80 supports the flag +hpxstd98 to get ANSI C++98
  # template support. It is known that version 6.25 doesn't need that flag.
  # Versions prior to 3.80 will not be able to build CMake. Current assumption:
  # it is needed for every version from 3.80 to 4 to get it working.
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4 AND
         NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.80)
    # use new C++ library and improved template support
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -AA +hpxstd98")
  endif()
endif()

# Workaround for short jump tables on PA-RISC
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^parisc")
  if(CMAKE_COMPILER_IS_GNUC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlong-calls")
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlong-calls")
  endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL SunPro)
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.13)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -library=stlport4")
  endif()
endif()

# use the ansi CXX compile flag for building cmake
if (CMAKE_ANSI_CXXFLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")
endif ()

if (CMAKE_ANSI_CFLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")
endif ()
