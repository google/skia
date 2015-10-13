
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

# Block multiple inclusion because "CMakeCInformation.cmake" includes
# "Platform/${CMAKE_SYSTEM_NAME}" even though the generic module
# "CMakeSystemSpecificInformation.cmake" already included it.
# The extra inclusion is a work-around documented next to the include()
# call, so this can be removed when the work-around is removed.
if(__WINDOWS_PATHS_INCLUDED)
  return()
endif()
set(__WINDOWS_PATHS_INCLUDED 1)

# Add the program-files folder(s) to the list of installation
# prefixes.
#
# Windows 64-bit Binary:
#   ENV{ProgramFiles(x86)} = [C:\Program Files (x86)]
#   ENV{ProgramFiles} = [C:\Program Files]
#   ENV{ProgramW6432} = <not set>
# (executed from cygwin):
#   ENV{ProgramFiles(x86)} = <not set>
#   ENV{ProgramFiles} = [C:\Program Files]
#   ENV{ProgramW6432} = <not set>
#
# Windows 32-bit Binary:
#   ENV{ProgramFiles(x86)} = [C:\Program Files (x86)]
#   ENV{ProgramFiles} = [C:\Program Files (x86)]
#   ENV{ProgramW6432} = [C:\Program Files]
# (executed from cygwin):
#   ENV{ProgramFiles(x86)} = <not set>
#   ENV{ProgramFiles} = [C:\Program Files (x86)]
#   ENV{ProgramW6432} = [C:\Program Files]
if(DEFINED "ENV{ProgramW6432}")
  # 32-bit binary on 64-bit windows.
  # The 64-bit program files are in ProgramW6432.
  list(APPEND CMAKE_SYSTEM_PREFIX_PATH "$ENV{ProgramW6432}")

  # The 32-bit program files are in ProgramFiles.
  if(DEFINED "ENV{ProgramFiles}")
    list(APPEND CMAKE_SYSTEM_PREFIX_PATH "$ENV{ProgramFiles}")
  endif()
else()
  # 64-bit binary, or 32-bit binary on 32-bit windows.
  if(DEFINED "ENV{ProgramFiles}")
    list(APPEND CMAKE_SYSTEM_PREFIX_PATH "$ENV{ProgramFiles}")
  endif()
  set(programfilesx86 "ProgramFiles(x86)")
  if(DEFINED "ENV{${programfilesx86}}")
    # 64-bit binary.  32-bit program files are in ProgramFiles(x86).
    list(APPEND CMAKE_SYSTEM_PREFIX_PATH "$ENV{${programfilesx86}}")
  elseif(DEFINED "ENV{SystemDrive}")
    # Guess the 32-bit program files location.
    if(EXISTS "$ENV{SystemDrive}/Program Files (x86)")
      list(APPEND CMAKE_SYSTEM_PREFIX_PATH
        "$ENV{SystemDrive}/Program Files (x86)")
    endif()
  endif()
endif()

# Add the CMake install location.
get_filename_component(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
get_filename_component(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)
list(APPEND CMAKE_SYSTEM_PREFIX_PATH "${_CMAKE_INSTALL_DIR}")

if (NOT CMAKE_FIND_NO_INSTALL_PREFIX)
  # Add other locations.
  list(APPEND CMAKE_SYSTEM_PREFIX_PATH
    # Project install destination.
    "${CMAKE_INSTALL_PREFIX}"
    )
  if (CMAKE_STAGING_PREFIX)
    list(APPEND CMAKE_SYSTEM_PREFIX_PATH
      # User-supplied staging prefix.
      "${CMAKE_STAGING_PREFIX}"
    )
  endif()
endif()

if(CMAKE_CROSSCOMPILING AND NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
  # MinGW (useful when cross compiling from linux with CMAKE_FIND_ROOT_PATH set)
  list(APPEND CMAKE_SYSTEM_PREFIX_PATH /)
endif()

list(APPEND CMAKE_SYSTEM_INCLUDE_PATH
  )

# mingw can also link against dlls which can also be in /bin, so list this too
if (NOT CMAKE_FIND_NO_INSTALL_PREFIX)
  list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
    "${CMAKE_INSTALL_PREFIX}/bin"
  )
  if (CMAKE_STAGING_PREFIX)
    list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
      "${CMAKE_STAGING_PREFIX}/bin"
    )
  endif()
endif()
list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  "${_CMAKE_INSTALL_DIR}/bin"
  /bin
  )

list(APPEND CMAKE_SYSTEM_PROGRAM_PATH
  )
