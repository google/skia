#.rst:
# FindGit
# -------
#
#
#
# The module defines the following variables:
#
# ::
#
#    GIT_EXECUTABLE - path to git command line client
#    GIT_FOUND - true if the command line client was found
#    GIT_VERSION_STRING - the version of git found (since CMake 2.8.8)
#
# Example usage:
#
# ::
#
#    find_package(Git)
#    if(GIT_FOUND)
#      message("git found: ${GIT_EXECUTABLE}")
#    endif()

#=============================================================================
# Copyright 2010 Kitware, Inc.
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
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

# Look for 'git' or 'eg' (easy git)
#
set(git_names git eg)

# Prefer .cmd variants on Windows unless running in a Makefile
# in the MSYS shell.
#
if(WIN32)
  if(NOT CMAKE_GENERATOR MATCHES "MSYS")
    set(git_names git.cmd git eg.cmd eg)
    # GitHub search path for Windows
    set(github_path "$ENV{LOCALAPPDATA}/Github/PortableGit*/bin")
    file(GLOB github_path "${github_path}")
  endif()
endif()

find_program(GIT_EXECUTABLE
  NAMES ${git_names}
  PATHS ${github_path}
  PATH_SUFFIXES Git/cmd Git/bin
  DOC "git command line client"
  )
mark_as_advanced(GIT_EXECUTABLE)

if(GIT_EXECUTABLE)
  execute_process(COMMAND ${GIT_EXECUTABLE} --version
                  OUTPUT_VARIABLE git_version
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (git_version MATCHES "^git version [0-9]")
    string(REPLACE "git version " "" GIT_VERSION_STRING "${git_version}")
  endif()
  unset(git_version)
endif()

# Handle the QUIETLY and REQUIRED arguments and set GIT_FOUND to TRUE if
# all listed variables are TRUE

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Git
                                  REQUIRED_VARS GIT_EXECUTABLE
                                  VERSION_VAR GIT_VERSION_STRING)
