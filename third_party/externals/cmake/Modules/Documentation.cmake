#.rst:
# Documentation
# -------------
#
# DocumentationVTK.cmake
#
# This file provides support for the VTK documentation framework.  It
# relies on several tools (Doxygen, Perl, etc).

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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

#
# Build the documentation ?
#
option(BUILD_DOCUMENTATION "Build the documentation (Doxygen)." OFF)
mark_as_advanced(BUILD_DOCUMENTATION)

if (BUILD_DOCUMENTATION)

  #
  # Check for the tools
  #
  find_package(UnixCommands)
  find_package(Doxygen)
  find_package(Gnuplot)
  find_package(HTMLHelp)
  find_package(Perl)
  find_package(Wget)

  option(DOCUMENTATION_HTML_HELP
    "Build the HTML Help file (CHM)." OFF)

  option(DOCUMENTATION_HTML_TARZ
    "Build a compressed tar archive of the HTML doc." OFF)

  mark_as_advanced(
    DOCUMENTATION_HTML_HELP
    DOCUMENTATION_HTML_TARZ
    )

  #
  # The documentation process is controled by a batch file.
  # We will probably need bash to create the custom target
  #

endif ()
