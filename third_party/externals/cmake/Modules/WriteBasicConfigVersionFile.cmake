#.rst:
# WriteBasicConfigVersionFile
# ---------------------------
#
#
#
# ::
#
#   WRITE_BASIC_CONFIG_VERSION_FILE( filename
#     [VERSION major.minor.patch]
#     COMPATIBILITY (AnyNewerVersion|SameMajorVersion)
#     )
#
#
#
# Deprecated, see WRITE_BASIC_PACKAGE_VERSION_FILE(), it is identical.

#=============================================================================
# Copyright 2008-2011 Alexander Neundorf, <neundorf@kde.org>
# Copyright 2004-2009 Kitware, Inc.
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

include(CMakeParseArguments)

function(WRITE_BASIC_CONFIG_VERSION_FILE _filename)

  set(options )
  set(oneValueArgs VERSION COMPATIBILITY )
  set(multiValueArgs )

  cmake_parse_arguments(CVF "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

  if(CVF_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to WRITE_BASIC_CONFIG_VERSION_FILE(): \"${CVF_UNPARSED_ARGUMENTS}\"")
  endif()

  set(versionTemplateFile "${CMAKE_ROOT}/Modules/BasicConfigVersion-${CVF_COMPATIBILITY}.cmake.in")
  if(NOT EXISTS "${versionTemplateFile}")
    message(FATAL_ERROR "Bad COMPATIBILITY value used for WRITE_BASIC_CONFIG_VERSION_FILE(): \"${CVF_COMPATIBILITY}\"")
  endif()

  if("${CVF_VERSION}" STREQUAL "")
    if ("${PROJECT_VERSION}" STREQUAL "")
      message(FATAL_ERROR "No VERSION specified for WRITE_BASIC_CONFIG_VERSION_FILE()")
    else()
      set(CVF_VERSION "${PROJECT_VERSION}")
    endif()
  endif()

  configure_file("${versionTemplateFile}" "${_filename}" @ONLY)

endfunction()
