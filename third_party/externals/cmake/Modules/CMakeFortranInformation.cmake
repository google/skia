
#=============================================================================
# Copyright 2004-2011 Kitware, Inc.
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

# This file sets the basic flags for the Fortran language in CMake.
# It also loads the available platform file for the system-compiler
# if it exists.

set(_INCLUDED_FILE 0)

# Load compiler-specific information.
if(CMAKE_Fortran_COMPILER_ID)
  include(Compiler/${CMAKE_Fortran_COMPILER_ID}-Fortran OPTIONAL)
endif()

set(CMAKE_BASE_NAME)
get_filename_component(CMAKE_BASE_NAME "${CMAKE_Fortran_COMPILER}" NAME_WE)
# since the gnu compiler has several names force g++
if(CMAKE_COMPILER_IS_GNUG77)
  set(CMAKE_BASE_NAME g77)
endif()
if(CMAKE_Fortran_COMPILER_ID)
  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_Fortran_COMPILER_ID}-Fortran OPTIONAL RESULT_VARIABLE _INCLUDED_FILE)
endif()
if (NOT _INCLUDED_FILE)
  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME} OPTIONAL
          RESULT_VARIABLE _INCLUDED_FILE)
endif ()
# We specify the compiler information in the system file for some
# platforms, but this language may not have been enabled when the file
# was first included.  Include it again to get the language info.
# Remove this when all compiler info is removed from system files.
if (NOT _INCLUDED_FILE)
  include(Platform/${CMAKE_SYSTEM_NAME} OPTIONAL)
endif ()

if(CMAKE_Fortran_SIZEOF_DATA_PTR)
  foreach(f ${CMAKE_Fortran_ABI_FILES})
    include(${f})
  endforeach()
  unset(CMAKE_Fortran_ABI_FILES)
endif()

# This should be included before the _INIT variables are
# used to initialize the cache.  Since the rule variables
# have if blocks on them, users can still define them here.
# But, it should still be after the platform file so changes can
# be made to those values.

if(CMAKE_USER_MAKE_RULES_OVERRIDE)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE "${_override}")
endif()

if(CMAKE_USER_MAKE_RULES_OVERRIDE_Fortran)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE_Fortran} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE_Fortran "${_override}")
endif()


# Fortran needs cmake to do a requires step during its build process to
# catch any modules
set(CMAKE_NEEDS_REQUIRES_STEP_Fortran_FLAG 1)

if(NOT CMAKE_Fortran_COMPILE_OPTIONS_PIC)
  set(CMAKE_Fortran_COMPILE_OPTIONS_PIC ${CMAKE_C_COMPILE_OPTIONS_PIC})
endif()

if(NOT CMAKE_Fortran_COMPILE_OPTIONS_PIE)
  set(CMAKE_Fortran_COMPILE_OPTIONS_PIE ${CMAKE_C_COMPILE_OPTIONS_PIE})
endif()

if(NOT CMAKE_Fortran_COMPILE_OPTIONS_DLL)
  set(CMAKE_Fortran_COMPILE_OPTIONS_DLL ${CMAKE_C_COMPILE_OPTIONS_DLL})
endif()

# Create a set of shared library variable specific to Fortran
# For 90% of the systems, these are the same flags as the C versions
# so if these are not set just copy the flags from the c version
if(NOT DEFINED CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS)
  set(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_Fortran_FLAGS)
  set(CMAKE_SHARED_LIBRARY_Fortran_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_LINK_Fortran_FLAGS)
  set(CMAKE_SHARED_LIBRARY_LINK_Fortran_FLAGS ${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_RUNTIME_Fortran_FLAG)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_Fortran_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_RUNTIME_Fortran_FLAG_SEP)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_Fortran_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_RPATH_LINK_Fortran_FLAG)
  set(CMAKE_SHARED_LIBRARY_RPATH_LINK_Fortran_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG})
endif()

if(NOT DEFINED CMAKE_EXE_EXPORTS_Fortran_FLAG)
  set(CMAKE_EXE_EXPORTS_Fortran_FLAG ${CMAKE_EXE_EXPORTS_C_FLAG})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG)
  set(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG ${CMAKE_SHARED_LIBRARY_SONAME_C_FLAG})
endif()

# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
if(NOT CMAKE_MODULE_EXISTS)
  set(CMAKE_SHARED_MODULE_Fortran_FLAGS ${CMAKE_SHARED_LIBRARY_Fortran_FLAGS})
  set(CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS})
endif()

# repeat for modules
if(NOT DEFINED CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS)
  set(CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS ${CMAKE_SHARED_MODULE_CREATE_C_FLAGS})
endif()

if(NOT DEFINED CMAKE_SHARED_MODULE_Fortran_FLAGS)
  set(CMAKE_SHARED_MODULE_Fortran_FLAGS ${CMAKE_SHARED_MODULE_C_FLAGS})
endif()

if(NOT DEFINED CMAKE_EXECUTABLE_RUNTIME_Fortran_FLAG)
  set(CMAKE_EXECUTABLE_RUNTIME_Fortran_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_Fortran_FLAG})
endif()

if(NOT DEFINED CMAKE_EXECUTABLE_RUNTIME_Fortran_FLAG_SEP)
  set(CMAKE_EXECUTABLE_RUNTIME_Fortran_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_Fortran_FLAG_SEP})
endif()

if(NOT DEFINED CMAKE_EXECUTABLE_RPATH_LINK_Fortran_FLAG)
  set(CMAKE_EXECUTABLE_RPATH_LINK_Fortran_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_Fortran_FLAG})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_LINK_Fortran_WITH_RUNTIME_PATH)
  set(CMAKE_SHARED_LIBRARY_LINK_Fortran_WITH_RUNTIME_PATH ${CMAKE_SHARED_LIBRARY_LINK_C_WITH_RUNTIME_PATH})
endif()

if(NOT CMAKE_INCLUDE_FLAG_Fortran)
  set(CMAKE_INCLUDE_FLAG_Fortran ${CMAKE_INCLUDE_FLAG_C})
endif()

if(NOT CMAKE_INCLUDE_FLAG_SEP_Fortran)
  set(CMAKE_INCLUDE_FLAG_SEP_Fortran ${CMAKE_INCLUDE_FLAG_SEP_C})
endif()

set(CMAKE_VERBOSE_MAKEFILE FALSE CACHE BOOL "If this value is on, makefiles will be generated without the .SILENT directive, and all commands will be echoed to the console during the make.  This is useful for debugging only. With Visual Studio IDE projects all commands are done without /nologo.")

set(CMAKE_Fortran_FLAGS_INIT "$ENV{FFLAGS} ${CMAKE_Fortran_FLAGS_INIT}")
# avoid just having a space as the initial value for the cache
if(CMAKE_Fortran_FLAGS_INIT STREQUAL " ")
  set(CMAKE_Fortran_FLAGS_INIT)
endif()
set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS_INIT}" CACHE STRING
     "Flags for Fortran compiler.")

include(CMakeCommonLanguageInclude)

# now define the following rule variables
# CMAKE_Fortran_CREATE_SHARED_LIBRARY
# CMAKE_Fortran_CREATE_SHARED_MODULE
# CMAKE_Fortran_COMPILE_OBJECT
# CMAKE_Fortran_LINK_EXECUTABLE

# create a Fortran shared library
if(NOT CMAKE_Fortran_CREATE_SHARED_LIBRARY)
  set(CMAKE_Fortran_CREATE_SHARED_LIBRARY
      "<CMAKE_Fortran_COMPILER> <CMAKE_SHARED_LIBRARY_Fortran_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
endif()

# create a Fortran shared module just copy the shared library rule
if(NOT CMAKE_Fortran_CREATE_SHARED_MODULE)
  set(CMAKE_Fortran_CREATE_SHARED_MODULE ${CMAKE_Fortran_CREATE_SHARED_LIBRARY})
endif()

# Create a static archive incrementally for large object file counts.
# If CMAKE_Fortran_CREATE_STATIC_LIBRARY is set it will override these.
if(NOT DEFINED CMAKE_Fortran_ARCHIVE_CREATE)
  set(CMAKE_Fortran_ARCHIVE_CREATE "<CMAKE_AR> cq <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_Fortran_ARCHIVE_APPEND)
  set(CMAKE_Fortran_ARCHIVE_APPEND "<CMAKE_AR> q  <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_Fortran_ARCHIVE_FINISH)
  set(CMAKE_Fortran_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
endif()

# compile a Fortran file into an object file
# (put -o after -c to workaround bug in at least one mpif77 wrapper)
if(NOT CMAKE_Fortran_COMPILE_OBJECT)
  set(CMAKE_Fortran_COMPILE_OBJECT
    "<CMAKE_Fortran_COMPILER> <DEFINES> <FLAGS> -c <SOURCE> -o <OBJECT>")
endif()

# link a fortran program
if(NOT CMAKE_Fortran_LINK_EXECUTABLE)
  set(CMAKE_Fortran_LINK_EXECUTABLE
    "<CMAKE_Fortran_COMPILER> <CMAKE_Fortran_LINK_FLAGS> <LINK_FLAGS> <FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
endif()

if(CMAKE_Fortran_STANDARD_LIBRARIES_INIT)
  set(CMAKE_Fortran_STANDARD_LIBRARIES "${CMAKE_Fortran_STANDARD_LIBRARIES_INIT}"
    CACHE STRING "Libraries linked by default with all Fortran applications.")
  mark_as_advanced(CMAKE_Fortran_STANDARD_LIBRARIES)
endif()

if(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
  set (CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG_INIT}" CACHE STRING
     "Flags used by the compiler during debug builds.")
  set (CMAKE_Fortran_FLAGS_MINSIZEREL "${CMAKE_Fortran_FLAGS_MINSIZEREL_INIT}" CACHE STRING
     "Flags used by the compiler during release builds for minimum size.")
  set (CMAKE_Fortran_FLAGS_RELEASE "${CMAKE_Fortran_FLAGS_RELEASE_INIT}" CACHE STRING
     "Flags used by the compiler during release builds.")
  set (CMAKE_Fortran_FLAGS_RELWITHDEBINFO "${CMAKE_Fortran_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
     "Flags used by the compiler during release builds with debug info.")

endif()

mark_as_advanced(
CMAKE_Fortran_FLAGS
CMAKE_Fortran_FLAGS_DEBUG
CMAKE_Fortran_FLAGS_MINSIZEREL
CMAKE_Fortran_FLAGS_RELEASE
CMAKE_Fortran_FLAGS_RELWITHDEBINFO)

# set this variable so we can avoid loading this more than once.
set(CMAKE_Fortran_INFORMATION_LOADED 1)
