#.rst:
# Findosg
# -------
#
#
#
#
#
# NOTE: It is highly recommended that you use the new
# FindOpenSceneGraph.cmake introduced in CMake 2.6.3 and not use this
# Find module directly.
#
# This is part of the Findosg* suite used to find OpenSceneGraph
# components.  Each component is separate and you must opt in to each
# module.  You must also opt into OpenGL and OpenThreads (and Producer
# if needed) as these modules won't do it for you.  This is to allow you
# control over your own system piece by piece in case you need to opt
# out of certain components or change the Find behavior for a particular
# module (perhaps because the default FindOpenGL.cmake module doesn't
# work with your system as an example).  If you want to use a more
# convenient module that includes everything, use the
# FindOpenSceneGraph.cmake instead of the Findosg*.cmake modules.
#
# Locate osg This module defines
#
# OSG_FOUND - Was the Osg found? OSG_INCLUDE_DIR - Where to find the
# headers OSG_LIBRARIES - The libraries to link against for the OSG (use
# this)
#
# OSG_LIBRARY - The OSG library OSG_LIBRARY_DEBUG - The OSG debug
# library
#
# $OSGDIR is an environment variable that would correspond to the
# ./configure --prefix=$OSGDIR used in building osg.
#
# Created by Eric Wing.

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
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

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgUtil/SceneView>

include(${CMAKE_CURRENT_LIST_DIR}/Findosg_functions.cmake)
OSG_FIND_PATH   (OSG osg/PositionAttitudeTransform)
OSG_FIND_LIBRARY(OSG osg)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(osg DEFAULT_MSG OSG_LIBRARY OSG_INCLUDE_DIR)
