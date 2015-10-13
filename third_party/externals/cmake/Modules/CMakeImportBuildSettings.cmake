
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

# This module is purposely no longer documented.  It does nothing useful.

# This macro used to load build settings from another project that
# stored settings using the CMAKE_EXPORT_BUILD_SETTINGS macro.
macro(CMAKE_IMPORT_BUILD_SETTINGS SETTINGS_FILE)
  if("${SETTINGS_FILE}" STREQUAL "")
    message(SEND_ERROR "CMAKE_IMPORT_BUILD_SETTINGS called with no argument.")
  endif()
endmacro()
