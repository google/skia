#.rst:
# Use_wxWindows
# -------------
#
#
#
#
# This convenience include finds if wxWindows is installed and set the
# appropriate libs, incdirs, flags etc.  author Jan Woetzel <jw -at-
# mip.informatik.uni-kiel.de> (07/2003)
#
# USAGE:
#
# ::
#
#    just include Use_wxWindows.cmake
#    in your projects CMakeLists.txt
#
# include( ${CMAKE_MODULE_PATH}/Use_wxWindows.cmake)
#
# ::
#
#    if you are sure you need GL then
#
# set(WXWINDOWS_USE_GL 1)
#
# ::
#
#    *before* you include this file.

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
# Copyright 2003      Jan Woetzel
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

# -----------------------------------------------------
# 16.Feb.2004: changed INCLUDE to FIND_PACKAGE to read from users own non-system CMAKE_MODULE_PATH (Jan Woetzel JW)
# 07/2006: rewrite as FindwxWidgets.cmake, kept for backward compatibility JW

message(STATUS "Use_wxWindows.cmake is DEPRECATED. \n"
"Please use find_package(wxWidgets) and include(${wxWidgets_USE_FILE}) instead. (JW)")


# ------------------------

find_package( wxWindows )

if(WXWINDOWS_FOUND)

#message("DBG Use_wxWindows.cmake:  WXWINDOWS_INCLUDE_DIR=${WXWINDOWS_INCLUDE_DIR} WXWINDOWS_LINK_DIRECTORIES=${WXWINDOWS_LINK_DIRECTORIES}     WXWINDOWS_LIBRARIES=${WXWINDOWS_LIBRARIES}  CMAKE_WXWINDOWS_CXX_FLAGS=${CMAKE_WXWINDOWS_CXX_FLAGS} WXWINDOWS_DEFINITIONS=${WXWINDOWS_DEFINITIONS}")

 if(WXWINDOWS_INCLUDE_DIR)
    include_directories(${WXWINDOWS_INCLUDE_DIR})
  endif()
 if(WXWINDOWS_LINK_DIRECTORIES)
    link_directories(${WXWINDOWS_LINK_DIRECTORIES})
  endif()
  if(WXWINDOWS_LIBRARIES)
    link_libraries(${WXWINDOWS_LIBRARIES})
  endif()
  if (CMAKE_WXWINDOWS_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_WXWINDOWS_CXX_FLAGS}")
  endif()
  if(WXWINDOWS_DEFINITIONS)
    add_definitions(${WXWINDOWS_DEFINITIONS})
  endif()
else()
  message(SEND_ERROR "wxWindows not found by Use_wxWindows.cmake")
endif()

