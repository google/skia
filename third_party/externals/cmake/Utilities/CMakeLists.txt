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
subdirs(Doxygen KWStyle)

if(CMAKE_DOC_TARBALL)
  # Undocumented option to extract and install pre-built documentation.
  # This is intended for use during packaging of CMake itself.
  if(CMAKE_DOC_TARBALL MATCHES "/([^/]+)\\.tar\\.gz$")
    set(dir "${CMAKE_MATCH_1}")
  else()
    message(FATAL_ERROR "CMAKE_DOC_TARBALL must end in .tar.gz")
  endif()
  add_custom_command(
    OUTPUT ${dir}.stamp
    COMMAND cmake -E remove_directory ${dir}
    COMMAND cmake -E tar xf ${CMAKE_DOC_TARBALL}
    COMMAND cmake -E touch ${dir}.stamp
    DEPENDS ${CMAKE_DOC_TARBALL}
    )
  add_custom_target(documentation ALL DEPENDS ${dir}.stamp)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${dir}/
          DESTINATION . USE_SOURCE_PERMISSIONS)
else()
  # Normal documentation build.
  add_subdirectory(Sphinx)
endif()
