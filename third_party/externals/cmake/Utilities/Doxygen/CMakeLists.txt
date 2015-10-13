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

#
# Build the documentation
#
include (${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

if (BUILD_DOCUMENTATION)

  #
  # Configure the script and the doxyfile, then add target
  #
  if(NOT DOT_PATH)
    get_filename_component(DOT_PATH ${DOT} PATH)
  endif()

  configure_file(
    ${CMake_SOURCE_DIR}/Utilities/Doxygen/doxyfile.in
    ${CMake_BINARY_DIR}/Utilities/Doxygen/doxyfile)

  configure_file(
    ${CMake_SOURCE_DIR}/Utilities/Doxygen/doc_makeall.sh.in
    ${CMake_BINARY_DIR}/Utilities/Doxygen/doc_makeall.sh)

  add_custom_target(DoxygenDoc
    ${BASH}
    ${CMake_BINARY_DIR}/Utilities/Doxygen/doc_makeall.sh)

endif ()
