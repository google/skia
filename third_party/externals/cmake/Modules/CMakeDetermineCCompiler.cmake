
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

# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.
# use environment variable CC first if defined by user, next use
# the cmake variable CMAKE_GENERATOR_CC which can be defined by a generator
# as a default compiler
# If the internal cmake variable _CMAKE_TOOLCHAIN_PREFIX is set, this is used
# as prefix for the tools (e.g. arm-elf-gcc, arm-elf-ar etc.). This works
# currently with the GNU crosscompilers.
#
# Sets the following variables:
#   CMAKE_C_COMPILER
#   CMAKE_AR
#   CMAKE_RANLIB
#   CMAKE_COMPILER_IS_GNUCC
#
# If not already set before, it also sets
#   _CMAKE_TOOLCHAIN_PREFIX

include(${CMAKE_ROOT}/Modules/CMakeDetermineCompiler.cmake)

# Load system-specific compiler preferences for this language.
include(Platform/${CMAKE_SYSTEM_NAME}-C OPTIONAL)
if(NOT CMAKE_C_COMPILER_NAMES)
  set(CMAKE_C_COMPILER_NAMES cc)
endif()

if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
elseif("${CMAKE_GENERATOR}" MATCHES "Xcode")
  set(CMAKE_C_COMPILER_XCODE_TYPE sourcecode.c.c)
  _cmake_find_compiler_path(C)
else()
  if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER_INIT NOTFOUND)

    # prefer the environment variable CC
    if(NOT $ENV{CC} STREQUAL "")
      get_filename_component(CMAKE_C_COMPILER_INIT $ENV{CC} PROGRAM PROGRAM_ARGS CMAKE_C_FLAGS_ENV_INIT)
      if(CMAKE_C_FLAGS_ENV_INIT)
        set(CMAKE_C_COMPILER_ARG1 "${CMAKE_C_FLAGS_ENV_INIT}" CACHE STRING "First argument to C compiler")
      endif()
      if(NOT EXISTS ${CMAKE_C_COMPILER_INIT})
        message(FATAL_ERROR "Could not find compiler set in environment variable CC:\n$ENV{CC}.")
      endif()
    endif()

    # next try prefer the compiler specified by the generator
    if(CMAKE_GENERATOR_CC)
      if(NOT CMAKE_C_COMPILER_INIT)
        set(CMAKE_C_COMPILER_INIT ${CMAKE_GENERATOR_CC})
      endif()
    endif()

    # finally list compilers to try
    if(NOT CMAKE_C_COMPILER_INIT)
      set(CMAKE_C_COMPILER_LIST ${_CMAKE_TOOLCHAIN_PREFIX}cc ${_CMAKE_TOOLCHAIN_PREFIX}gcc cl bcc xlc clang)
    endif()

    _cmake_find_compiler(C)

  else()
    _cmake_find_compiler_path(C)
  endif()
  mark_as_advanced(CMAKE_C_COMPILER)

  # Each entry in this list is a set of extra flags to try
  # adding to the compile line to see if it helps produce
  # a valid identification file.
  set(CMAKE_C_COMPILER_ID_TEST_FLAGS
    # Try compiling to an object file only.
    "-c"

    # Try enabling ANSI mode on HP.
    "-Aa"
    )
endif()

# Build a small source file to identify the compiler.
if(NOT CMAKE_C_COMPILER_ID_RUN)
  set(CMAKE_C_COMPILER_ID_RUN 1)

  # Try to identify the compiler.
  set(CMAKE_C_COMPILER_ID)
  file(READ ${CMAKE_ROOT}/Modules/CMakePlatformId.h.in
    CMAKE_C_COMPILER_ID_PLATFORM_CONTENT)

  # The IAR compiler produces weird output.
  # See http://www.cmake.org/Bug/view.php?id=10176#c19598
  list(APPEND CMAKE_C_COMPILER_ID_VENDORS IAR)
  set(CMAKE_C_COMPILER_ID_VENDOR_FLAGS_IAR )
  set(CMAKE_C_COMPILER_ID_VENDOR_REGEX_IAR "IAR .+ Compiler")

  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(C CFLAGS CMakeCCompilerId.c)

  # Set old compiler and platform id variables.
  if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_COMPILER_IS_GNUCC 1)
  endif()
  if("${CMAKE_C_PLATFORM_ID}" MATCHES "MinGW")
    set(CMAKE_COMPILER_IS_MINGW 1)
  elseif("${CMAKE_C_PLATFORM_ID}" MATCHES "Cygwin")
    set(CMAKE_COMPILER_IS_CYGWIN 1)
  endif()
endif()

if (NOT _CMAKE_TOOLCHAIN_LOCATION)
  get_filename_component(_CMAKE_TOOLCHAIN_LOCATION "${CMAKE_C_COMPILER}" PATH)
endif ()

# If we have a gcc cross compiler, they have usually some prefix, like
# e.g. powerpc-linux-gcc, arm-elf-gcc or i586-mingw32msvc-gcc, optionally
# with a 3-component version number at the end (e.g. arm-eabi-gcc-4.5.2).
# The other tools of the toolchain usually have the same prefix
# NAME_WE cannot be used since then this test will fail for names like
# "arm-unknown-nto-qnx6.3.0-gcc.exe", where BASENAME would be
# "arm-unknown-nto-qnx6" instead of the correct "arm-unknown-nto-qnx6.3.0-"
if (CMAKE_CROSSCOMPILING  AND NOT _CMAKE_TOOLCHAIN_PREFIX)

  if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    get_filename_component(COMPILER_BASENAME "${CMAKE_C_COMPILER}" NAME)
    if (COMPILER_BASENAME MATCHES "^(.+-)(clang|g?cc)(-[0-9]+\\.[0-9]+\\.[0-9]+)?(\\.exe)?$")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
      if(CMAKE_C_COMPILER_TARGET)
        set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_C_COMPILER_TARGET}-)
      endif()
    elseif(COMPILER_BASENAME MATCHES "qcc(\\.exe)?$")
      if(CMAKE_C_COMPILER_TARGET MATCHES "gcc_nto([^_le]+)(le)?")
        set(_CMAKE_TOOLCHAIN_PREFIX nto${CMAKE_MATCH_1}-)
      endif()
    endif ()

    # if "llvm-" is part of the prefix, remove it, since llvm doesn't have its own binutils
    # but uses the regular ar, objcopy, etc. (instead of llvm-objcopy etc.)
    if ("${_CMAKE_TOOLCHAIN_PREFIX}" MATCHES "(.+-)?llvm-$")
      set(_CMAKE_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
    endif ()
  elseif(CMAKE_C_COMPILER_ID MATCHES "TI")
    # TI compilers are named e.g. cl6x, cl470 or armcl.exe
    get_filename_component(COMPILER_BASENAME "${CMAKE_C_COMPILER}" NAME)
    if (COMPILER_BASENAME MATCHES "^(.+)?cl([^.]+)?(\\.exe)?$")
      set(_CMAKE_TOOLCHAIN_PREFIX "${CMAKE_MATCH_1}")
      set(_CMAKE_TOOLCHAIN_SUFFIX "${CMAKE_MATCH_2}")
    endif ()
  endif()

endif ()

include(CMakeFindBinUtils)
if(MSVC_C_ARCHITECTURE_ID)
  include(${CMAKE_ROOT}/Modules/CMakeClDeps.cmake)
  set(SET_MSVC_C_ARCHITECTURE_ID
    "set(MSVC_C_ARCHITECTURE_ID ${MSVC_C_ARCHITECTURE_ID})")
endif()

# configure variables set in this file for fast reload later on
configure_file(${CMAKE_ROOT}/Modules/CMakeCCompiler.cmake.in
  ${CMAKE_PLATFORM_INFO_DIR}/CMakeCCompiler.cmake
  @ONLY
  )
set(CMAKE_C_COMPILER_ENV_VAR "CC")
