
#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
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

# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.
# use environment variable CXX first if defined by user, next use
# the cmake variable CMAKE_GENERATOR_CXX which can be defined by a generator
# as a default compiler
# If the internal cmake variable _CMAKE_TOOLCHAIN_PREFIX is set, this is used
# as prefix for the tools (e.g. arm-elf-g++, arm-elf-ar etc.)
#
# Sets the following variables:
#   CMAKE_CXX_COMPILER
#   CMAKE_COMPILER_IS_GNUCXX
#   CMAKE_AR
#   CMAKE_RANLIB
#
# If not already set before, it also sets
#   _CMAKE_TOOLCHAIN_PREFIX

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)

# Load system-specific compiler preferences for this language.
include(Platform/${CMAKE_SYSTEM_NAME}-CXX OPTIONAL)
if(NOT CMAKE_CXX_COMPILER_NAMES)
  set(CMAKE_CXX_COMPILER_NAMES CC)
endif()

if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
elseif("${CMAKE_GENERATOR}" MATCHES "Xcode")
  set(CMAKE_CXX_COMPILER_XCODE_TYPE sourcecode.cpp.cpp)
  _cmake_find_compiler_path(CXX)
else()
  if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER_INIT NOTFOUND)

    # prefer the environment variable CXX
    if(NOT $ENV{CXX} STREQUAL "")
      get_filename_component(CMAKE_CXX_COMPILER_INIT $ENV{CXX} PROGRAM PROGRAM_ARGS CMAKE_CXX_FLAGS_ENV_INIT)
      if(CMAKE_CXX_FLAGS_ENV_INIT)
        set(CMAKE_CXX_COMPILER_ARG1 "${CMAKE_CXX_FLAGS_ENV_INIT}" CACHE STRING "First argument to CXX compiler")
      endif()
      if(NOT EXISTS ${CMAKE_CXX_COMPILER_INIT})
        message(FATAL_ERROR "Could not find compiler set in environment variable CXX:\n$ENV{CXX}.\n${CMAKE_CXX_COMPILER_INIT}")
      endif()
    endif()

    # next prefer the generator specified compiler
    if(CMAKE_GENERATOR_CXX)
      if(NOT CMAKE_CXX_COMPILER_INIT)
        set(CMAKE_CXX_COMPILER_INIT ${CMAKE_GENERATOR_CXX})
      endif()
    endif()

    # finally list compilers to try
    if(NOT CMAKE_CXX_COMPILER_INIT)
      set(CMAKE_CXX_COMPILER_LIST CC ${_CMAKE_TOOLCHAIN_PREFIX}c++ ${_CMAKE_TOOLCHAIN_PREFIX}g++ aCC cl bcc xlC clang++)
    endif()

    _cmake_find_compiler(CXX)
  else()
    _cmake_find_compiler_path(CXX)
  endif()
  mark_as_advanced(CMAKE_CXX_COMPILER)

  # Each entry in this list is a set of extra flags to try
  # adding to the compile line to see if it helps produce
  # a valid identification file.
  set(CMAKE_CXX_COMPILER_ID_TEST_FLAGS
    # Try compiling to an object file only.
    "-c"
    )
endif()

# Build a small source file to identify the compiler.
if(NOT CMAKE_CXX_COMPILER_ID_RUN)
  set(CMAKE_CXX_COMPILER_ID_RUN 1)

  # Try to identify the compiler.
  set(CMAKE_CXX_COMPILER_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_CXX_COMPILER_ID_PLATFORM_CONTENT)

  # The IAR compiler produces weird output.
  # See http://www.cmake.org/Bug/view.php?id=10176#c19598
  list(APPEND CMAKE_CXX_COMPILER_ID_VENDORS IAR)
  set(CMAKE_CXX_COMPILER_ID_VENDOR_FLAGS_IAR )
  set(CMAKE_CXX_COMPILER_ID_VENDOR_REGEX_IAR "IAR .+ Compiler")

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(CXX CXXFLAGS CMakeCXXCompilerId.cpp)

  # Set old compiler and platform id variables.
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCXX 1)
  endif()
  if("${CMAKE_CXX_PLATFORM_ID}" MATCHES "MinGW")
    set(CMAKE_COMPILER_IS_MINGW 1)
  elseif("${CMAKE_CXX_PLATFORM_ID}" MATCHES "Cygwin")
    set(CMAKE_COMPILER_IS_CYGWIN 1)
  endif()
endif()

if (NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_CXX_COMPILER}" PATH)
endif ()

# if we have a g++ cross compiler, they have usually some prefix, like
# e.g. powerpc-linux-g++, arm-elf-g++ or i586-mingw32msvc-g++ , optionally
# with a 3-component version number at the end (e.g. arm-eabi-gcc-4.5.2).
# The other tools of the toolchain usually have the same prefix
# NAME_WE cannot be used since then this test will fail for names like
# "arm-unknown-nto-qnx6.3.0-gcc.exe", where BASENAME would be
# "arm-unknown-nto-qnx6" instead of the correct "arm-unknown-nto-qnx6.3.0-"


if (CMAKE_CROSSCOMPILING  AND NOT  _CMAKE_TOOLCHAIN_PREFIX)

  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    get_filename_component(COMPILER_BASENAME "${CMAKE_CXX_COMPILER}" NAME)
    if (COMPILER_BASENAME MATCHES "^(.+-)(clan)?[gc]\\+\\+(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
    elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      if(CMAKE_CXX_COMPILER_TARGET)
        set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_CXX_COMPILER_TARGET}-)
      endif()
    elseif(COMPILER_BASENAME MATCHES "QCC(\\.exe)?$")
      if(CMAKE_CXX_COMPILER_TARGET MATCHES "gcc_nto([^_le]+)(le)?")
        set(_CMAKE_TOOLCHAIN_PREFIX nto${CMAKE_MATCH_1}-)
      endif()
    endif ()

    # if "llvm-" is part of the prefix, remove it, since llvm doesn't have its own binutils
    # but uses the regular ar, objcopy, etc. (instead of llvm-objcopy etc.)
    if ("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
    endif ()
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "TI")
    # TI compilers are named e.g. cl6x, cl470 or armcl.exe
    get_filename_component(COMPILER_BASENAME "${CMAKE_CXX_COMPILER}" NAME)
    if (COMPILER_BASENAME MATCHES "^(.+)?cl([^.]+)?(\\.exe)?$")
      set(_CMAKE_TOOLCHAIN_PREFIX "${CMAKE_MATCH_1}")
      set(_CMAKE_TOOLCHAIN_SUFFIX "${CMAKE_MATCH_2}")
    endif ()

  endif()

endif ()

include(CMakeFindBinUtils)
if(MSVC_CXX_ARCHITECTURE_ID)
  include(${CMAKE_ROOT}/Modules/CMakeClDeps.cmake)
  set(SET_MSVC_CXX_ARCHITECTURE_ID
    "set(MSVC_CXX_ARCHITECTURE_ID ${MSVC_CXX_ARCHITECTURE_ID})")
endif()

# configure all variables set in this file
configure_file(${CMAKE_ROOT}/Modules/CMakeCXXCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeCXXCompiler.cmake
  @ONLY
  )

set(CMAKE_CXX_COMPILER_ENV_VAR "CXX")
