#.rst:
# AddFileDependencies
# -------------------
#
# ADD_FILE_DEPENDENCIES(source_file depend_files...)
#
# Adds the given files as dependencies to source_file

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
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

macro(ADD_FILE_DEPENDENCIES _file)

   get_source_file_property(_deps ${_file} OBJECT_DEPENDS)
   if (_deps)
      set(_deps ${_deps} ${ARGN})
   else ()
      set(_deps ${ARGN})
   endif ()

   set_source_files_properties(${_file} PROPERTIES OBJECT_DEPENDS "${_deps}")

endmacro()
