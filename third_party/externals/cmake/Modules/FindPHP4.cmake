#.rst:
# FindPHP4
# --------
#
# Find PHP4
#
# This module finds if PHP4 is installed and determines where the
# include files and libraries are.  It also determines what the name of
# the library is.  This code sets the following variables:
#
# ::
#
#   PHP4_INCLUDE_PATH       = path to where php.h can be found
#   PHP4_EXECUTABLE         = full path to the php4 binary

#=============================================================================
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

set(PHP4_POSSIBLE_INCLUDE_PATHS
  /usr/include/php4
  /usr/local/include/php4
  /usr/include/php
  /usr/local/include/php
  /usr/local/apache/php
  )

set(PHP4_POSSIBLE_LIB_PATHS
  /usr/lib
  )

find_path(PHP4_FOUND_INCLUDE_PATH main/php.h
  ${PHP4_POSSIBLE_INCLUDE_PATHS})

if(PHP4_FOUND_INCLUDE_PATH)
  set(php4_paths "${PHP4_POSSIBLE_INCLUDE_PATHS}")
  foreach(php4_path Zend main TSRM)
    set(php4_paths ${php4_paths} "${PHP4_FOUND_INCLUDE_PATH}/${php4_path}")
  endforeach()
  set(PHP4_INCLUDE_PATH "${php4_paths}")
endif()

find_program(PHP4_EXECUTABLE NAMES php4 php )

mark_as_advanced(
  PHP4_EXECUTABLE
  PHP4_FOUND_INCLUDE_PATH
  )

if(APPLE)
# this is a hack for now
  set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
   "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -Wl,-flat_namespace")
  foreach(symbol
    __efree
    __emalloc
    __estrdup
    __object_init_ex
    __zend_get_parameters_array_ex
    __zend_list_find
    __zval_copy_ctor
    _add_property_zval_ex
    _alloc_globals
    _compiler_globals
    _convert_to_double
    _convert_to_long
    _zend_error
    _zend_hash_find
    _zend_register_internal_class_ex
    _zend_register_list_destructors_ex
    _zend_register_resource
    _zend_rsrc_list_get_rsrc_type
    _zend_wrong_param_count
    _zval_used_for_init
    )
    set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
      "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS},-U,${symbol}")
  endforeach()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PHP4 DEFAULT_MSG PHP4_EXECUTABLE PHP4_INCLUDE_PATH)
