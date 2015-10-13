
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

# Use of this file is deprecated, and is here for backwards compatibility with CMake 1.4
# GLU library is now found by FindOpenGL.cmake
#

message(STATUS
  "WARNING: you are using the obsolete 'GLU' package, please use 'OpenGL' instead")

include(${CMAKE_CURRENT_LIST_DIR}/FindOpenGL.cmake)

if (OPENGL_GLU_FOUND)
  set (GLU_LIBRARY ${OPENGL_LIBRARIES})
  set (GLU_INCLUDE_PATH ${OPENGL_INCLUDE_DIR})
endif ()

