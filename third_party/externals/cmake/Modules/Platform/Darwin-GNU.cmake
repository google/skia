
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

# This module is shared by multiple languages; use include blocker.
if(__DARWIN_COMPILER_GNU)
  return()
endif()
set(__DARWIN_COMPILER_GNU 1)

macro(__darwin_compiler_gnu lang)
  set(CMAKE_${lang}_VERBOSE_FLAG "-v -Wl,-v") # also tell linker to print verbose output
  # GNU does not have -shared on OS X
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-dynamiclib -Wl,-headerpad_max_install_names")
  set(CMAKE_SHARED_MODULE_CREATE_${lang}_FLAGS "-bundle -Wl,-headerpad_max_install_names")

  if(NOT CMAKE_${lang}_COMPILER_VERSION VERSION_LESS 4.3)
    set(CMAKE_${lang}_SYSTEM_FRAMEWORK_SEARCH_FLAG "-iframework ")
  endif()
endmacro()

macro(cmake_gnu_set_sysroot_flag lang)
  if(NOT DEFINED CMAKE_${lang}_SYSROOT_FLAG)
    set(_doc "${lang} compiler has -isysroot")
    message(STATUS "Checking whether ${_doc}")
    execute_process(
      COMMAND ${CMAKE_${lang}_COMPILER} "-v" "--help"
      OUTPUT_VARIABLE _gcc_help
      ERROR_VARIABLE _gcc_help
      )
    if("${_gcc_help}" MATCHES "isysroot")
      message(STATUS "Checking whether ${_doc} - yes")
      set(CMAKE_${lang}_SYSROOT_FLAG "-isysroot")
    else()
      message(STATUS "Checking whether ${_doc} - no")
      set(CMAKE_${lang}_SYSROOT_FLAG "")
    endif()
    set(CMAKE_${lang}_SYSROOT_FLAG_CODE "set(CMAKE_${lang}_SYSROOT_FLAG \"${CMAKE_${lang}_SYSROOT_FLAG}\")")
  endif()
endmacro()

macro(cmake_gnu_set_osx_deployment_target_flag lang)
  if(NOT DEFINED CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG)
    set(_doc "${lang} compiler supports OSX deployment target flag")
    message(STATUS "Checking whether ${_doc}")
    execute_process(
      COMMAND ${CMAKE_${lang}_COMPILER} "-v" "--help"
      OUTPUT_VARIABLE _gcc_help
      ERROR_VARIABLE _gcc_help
      )
    if("${_gcc_help}" MATCHES "macosx-version-min")
      message(STATUS "Checking whether ${_doc} - yes")
      set(CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG "-mmacosx-version-min=")
    else()
      message(STATUS "Checking whether ${_doc} - no")
      set(CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG "")
    endif()
    set(CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG_CODE "set(CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG \"${CMAKE_${lang}_OSX_DEPLOYMENT_TARGET_FLAG}\")")
  endif()
endmacro()
