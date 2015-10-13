#=============================================================================
# Kitware Information Macro Library
# Copyright 2010-2011 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# Import the KWIML directory tree into a subdirectory under a parent
# project and configure the library as follows:
#
#  set(KWIML myIML)
#  subdirs(KWIML)
#
# Optional settings are as follows:
#
#  KWIML_HEADER_ROOT = build tree directory to hold KWIML headers.
#    Headers will go in a directory called "${KWIML}" under this root.
#    For example:
#
#      set(KWIML_HEADER_ROOT ${PROJECT_BINARY_DIR})
#      include_directories(${PROJECT_BINARY_DIR})
#
#  KWIML_INSTALL_INCLUDE_DIR = install KWIML with "make install"
#    Specify a value relative to the install prefix and do NOT start with '/'.
#  KWIML_INSTALL_INCLUDE_OPTIONS = extra header installation options
#    Specify options for the install(FILES) command.
#
#  KWIML_LABELS_TEST = list of labels for KWIML tests

cmake_minimum_required(VERSION 2.6.3 FATAL_ERROR)

#-----------------------------------------------------------------------------
if(NOT DEFINED KWIML)
  if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    message(FATAL_ERROR "Set KWIML namespace in parent directory!")
  endif()
  set(KWIML KWIML)
  set(KWIML_STANDALONE 1)
  project(KWIML)
  include(CTest)
  mark_as_advanced(BUILD_TESTING)
endif()

#-----------------------------------------------------------------------------
get_property(KWIML_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(lang ${KWIML_LANGUAGES})
  set(KWIML_LANGUAGE_${lang} 1)
endforeach()
if(NOT KWIML_LANGUAGE_C AND NOT KWIML_LANGUAGE_CXX)
  set(BUILD_TESTING OFF)
endif()

#-----------------------------------------------------------------------------
if(NOT KWIML_HEADER_ROOT)
  set(KWIML_HEADER_ROOT "${PROJECT_BINARY_DIR}")
endif()
set(KWIML_HEADER_DIR "${KWIML_HEADER_ROOT}/${KWIML}")
include_directories(${KWIML_HEADER_ROOT})

#-----------------------------------------------------------------------------
foreach(h ABI INT)
  set(header ${KWIML_HEADER_DIR}/${h}.h)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${h}.h.in ${header} @ONLY)
  if(KWIML_INSTALL_INCLUDE_DIR)
    install(FILES ${header}
      DESTINATION ${KWIML_INSTALL_INCLUDE_DIR}/${KWIML}
      ${KWIML_INSTALL_INCLUDE_OPTIONS})
  endif()
endforeach()

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
