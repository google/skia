#.rst:
# FindHTMLHelp
# ------------
#
# This module looks for Microsoft HTML Help Compiler
#
# It defines:
#
# ::
#
#    HTML_HELP_COMPILER     : full path to the Compiler (hhc.exe)
#    HTML_HELP_INCLUDE_PATH : include path to the API (htmlhelp.h)
#    HTML_HELP_LIBRARY      : full path to the library (htmlhelp.lib)

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

if(WIN32)

  find_program(HTML_HELP_COMPILER
    hhc
    "[HKEY_CURRENT_USER\\Software\\Microsoft\\HTML Help Workshop;InstallDir]"
    "$ENV{ProgramFiles}/HTML Help Workshop"
    "C:/Program Files/HTML Help Workshop"
    )

  get_filename_component(HTML_HELP_COMPILER_PATH "${HTML_HELP_COMPILER}" PATH)

  find_path(HTML_HELP_INCLUDE_PATH
    htmlhelp.h
    "${HTML_HELP_COMPILER_PATH}/include"
    "[HKEY_CURRENT_USER\\Software\\Microsoft\\HTML Help Workshop;InstallDir]/include"
    "$ENV{ProgramFiles}/HTML Help Workshop/include"
    "C:/Program Files/HTML Help Workshop/include"
    )

  find_library(HTML_HELP_LIBRARY
    htmlhelp
    "${HTML_HELP_COMPILER_PATH}/lib"
    "[HKEY_CURRENT_USER\\Software\\Microsoft\\HTML Help Workshop;InstallDir]/lib"
    "$ENV{ProgramFiles}/HTML Help Workshop/lib"
    "C:/Program Files/HTML Help Workshop/lib"
    )

  mark_as_advanced(
    HTML_HELP_COMPILER
    HTML_HELP_INCLUDE_PATH
    HTML_HELP_LIBRARY
    )

endif()
