#.rst:
# FindPythonLibs
# --------------
#
# Find python libraries
#
# This module finds if Python is installed and determines where the
# include files and libraries are.  It also determines what the name of
# the library is.  This code sets the following variables:
#
# ::
#
#   PYTHONLIBS_FOUND           - have the Python libs been found
#   PYTHON_LIBRARIES           - path to the python library
#   PYTHON_INCLUDE_PATH        - path to where Python.h is found (deprecated)
#   PYTHON_INCLUDE_DIRS        - path to where Python.h is found
#   PYTHON_DEBUG_LIBRARIES     - path to the debug library (deprecated)
#   PYTHONLIBS_VERSION_STRING  - version of the Python libs found (since CMake 2.8.8)
#
#
#
# The Python_ADDITIONAL_VERSIONS variable can be used to specify a list
# of version numbers that should be taken into account when searching
# for Python.  You need to set this variable before calling
# find_package(PythonLibs).
#
# If you'd like to specify the installation of Python to use, you should
# modify the following cache variables:
#
# ::
#
#   PYTHON_LIBRARY             - path to the python library
#   PYTHON_INCLUDE_DIR         - path to where Python.h is found
#
# If also calling find_package(PythonInterp), call find_package(PythonInterp)
# first to get the currently active Python version by default with a consistent
# version of PYTHON_LIBRARIES.

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

include(${CMAKE_CURRENT_LIST_DIR}/CMakeFindFrameworks.cmake)
# Search for the python framework on Apple.
CMAKE_FIND_FRAMEWORKS(Python)

set(_PYTHON1_VERSIONS 1.6 1.5)
set(_PYTHON2_VERSIONS 2.7 2.6 2.5 2.4 2.3 2.2 2.1 2.0)
set(_PYTHON3_VERSIONS 3.4 3.3 3.2 3.1 3.0)

if(PythonLibs_FIND_VERSION)
    if(PythonLibs_FIND_VERSION_COUNT GREATER 1)
        set(_PYTHON_FIND_MAJ_MIN "${PythonLibs_FIND_VERSION_MAJOR}.${PythonLibs_FIND_VERSION_MINOR}")
        unset(_PYTHON_FIND_OTHER_VERSIONS)
        if(PythonLibs_FIND_VERSION_EXACT)
            if(_PYTHON_FIND_MAJ_MIN STREQUAL PythonLibs_FIND_VERSION)
                set(_PYTHON_FIND_OTHER_VERSIONS "${PythonLibs_FIND_VERSION}")
            else()
                set(_PYTHON_FIND_OTHER_VERSIONS "${PythonLibs_FIND_VERSION}" "${_PYTHON_FIND_MAJ_MIN}")
            endif()
        else()
            foreach(_PYTHON_V ${_PYTHON${PythonLibs_FIND_VERSION_MAJOR}_VERSIONS})
                if(NOT _PYTHON_V VERSION_LESS _PYTHON_FIND_MAJ_MIN)
                    list(APPEND _PYTHON_FIND_OTHER_VERSIONS ${_PYTHON_V})
                endif()
             endforeach()
        endif()
        unset(_PYTHON_FIND_MAJ_MIN)
    else()
        set(_PYTHON_FIND_OTHER_VERSIONS ${_PYTHON${PythonLibs_FIND_VERSION_MAJOR}_VERSIONS})
    endif()
else()
    set(_PYTHON_FIND_OTHER_VERSIONS ${_PYTHON3_VERSIONS} ${_PYTHON2_VERSIONS} ${_PYTHON1_VERSIONS})
endif()

# Set up the versions we know about, in the order we will search. Always add
# the user supplied additional versions to the front.
# If FindPythonInterp has already found the major and minor version,
# insert that version between the user supplied versions and the stock
# version list.
set(_Python_VERSIONS ${Python_ADDITIONAL_VERSIONS})
if(DEFINED PYTHON_VERSION_MAJOR AND DEFINED PYTHON_VERSION_MINOR)
  list(APPEND _Python_VERSIONS ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
endif()
list(APPEND _Python_VERSIONS ${_PYTHON_FIND_OTHER_VERSIONS})

unset(_PYTHON_FIND_OTHER_VERSIONS)
unset(_PYTHON1_VERSIONS)
unset(_PYTHON2_VERSIONS)
unset(_PYTHON3_VERSIONS)

foreach(_CURRENT_VERSION ${_Python_VERSIONS})
  string(REPLACE "." "" _CURRENT_VERSION_NO_DOTS ${_CURRENT_VERSION})
  if(WIN32)
    find_library(PYTHON_DEBUG_LIBRARY
      NAMES python${_CURRENT_VERSION_NO_DOTS}_d python
      PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs/Debug
      [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs/Debug
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
      [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
      )
  endif()

  find_library(PYTHON_LIBRARY
    NAMES
    python${_CURRENT_VERSION_NO_DOTS}
    python${_CURRENT_VERSION}mu
    python${_CURRENT_VERSION}m
    python${_CURRENT_VERSION}u
    python${_CURRENT_VERSION}
    PATHS
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
      [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/libs
    # Avoid finding the .dll in the PATH.  We want the .lib.
    NO_SYSTEM_ENVIRONMENT_PATH
  )
  # Look for the static library in the Python config directory
  find_library(PYTHON_LIBRARY
    NAMES python${_CURRENT_VERSION_NO_DOTS} python${_CURRENT_VERSION}
    # Avoid finding the .dll in the PATH.  We want the .lib.
    NO_SYSTEM_ENVIRONMENT_PATH
    # This is where the static library is usually located
    PATH_SUFFIXES python${_CURRENT_VERSION}/config
  )

  # For backward compatibility, honour value of PYTHON_INCLUDE_PATH, if
  # PYTHON_INCLUDE_DIR is not set.
  if(DEFINED PYTHON_INCLUDE_PATH AND NOT DEFINED PYTHON_INCLUDE_DIR)
    set(PYTHON_INCLUDE_DIR "${PYTHON_INCLUDE_PATH}" CACHE PATH
      "Path to where Python.h is found" FORCE)
  endif()

  set(PYTHON_FRAMEWORK_INCLUDES)
  if(Python_FRAMEWORKS AND NOT PYTHON_INCLUDE_DIR)
    foreach(dir ${Python_FRAMEWORKS})
      set(PYTHON_FRAMEWORK_INCLUDES ${PYTHON_FRAMEWORK_INCLUDES}
        ${dir}/Versions/${_CURRENT_VERSION}/include/python${_CURRENT_VERSION})
    endforeach()
  endif()

  find_path(PYTHON_INCLUDE_DIR
    NAMES Python.h
    PATHS
      ${PYTHON_FRAMEWORK_INCLUDES}
      [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/include
      [HKEY_CURRENT_USER\\SOFTWARE\\Python\\PythonCore\\${_CURRENT_VERSION}\\InstallPath]/include
    PATH_SUFFIXES
      python${_CURRENT_VERSION}mu
      python${_CURRENT_VERSION}m
      python${_CURRENT_VERSION}u
      python${_CURRENT_VERSION}
  )

  # For backward compatibility, set PYTHON_INCLUDE_PATH.
  set(PYTHON_INCLUDE_PATH "${PYTHON_INCLUDE_DIR}")

  if(PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}/patchlevel.h")
    file(STRINGS "${PYTHON_INCLUDE_DIR}/patchlevel.h" python_version_str
         REGEX "^#define[ \t]+PY_VERSION[ \t]+\"[^\"]+\"")
    string(REGEX REPLACE "^#define[ \t]+PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1"
                         PYTHONLIBS_VERSION_STRING "${python_version_str}")
    unset(python_version_str)
  endif()

  if(PYTHON_LIBRARY AND PYTHON_INCLUDE_DIR)
    break()
  endif()
endforeach()

mark_as_advanced(
  PYTHON_DEBUG_LIBRARY
  PYTHON_LIBRARY
  PYTHON_INCLUDE_DIR
)

# We use PYTHON_INCLUDE_DIR, PYTHON_LIBRARY and PYTHON_DEBUG_LIBRARY for the
# cache entries because they are meant to specify the location of a single
# library. We now set the variables listed by the documentation for this
# module.
set(PYTHON_INCLUDE_DIRS "${PYTHON_INCLUDE_DIR}")
set(PYTHON_DEBUG_LIBRARIES "${PYTHON_DEBUG_LIBRARY}")

# These variables have been historically named in this module different from
# what SELECT_LIBRARY_CONFIGURATIONS() expects.
set(PYTHON_LIBRARY_DEBUG "${PYTHON_DEBUG_LIBRARY}")
set(PYTHON_LIBRARY_RELEASE "${PYTHON_LIBRARY}")
include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
SELECT_LIBRARY_CONFIGURATIONS(PYTHON)
# SELECT_LIBRARY_CONFIGURATIONS() sets ${PREFIX}_FOUND if it has a library.
# Unset this, this prefix doesn't match the module prefix, they are different
# for historical reasons.
unset(PYTHON_FOUND)

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonLibs
                                  REQUIRED_VARS PYTHON_LIBRARIES PYTHON_INCLUDE_DIRS
                                  VERSION_VAR PYTHONLIBS_VERSION_STRING)

# PYTHON_ADD_MODULE(<name> src1 src2 ... srcN) is used to build modules for python.
# PYTHON_WRITE_MODULES_HEADER(<filename>) writes a header file you can include
# in your sources to initialize the static python modules
function(PYTHON_ADD_MODULE _NAME )
  get_property(_TARGET_SUPPORTS_SHARED_LIBS
    GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
  option(PYTHON_ENABLE_MODULE_${_NAME} "Add module ${_NAME}" TRUE)
  option(PYTHON_MODULE_${_NAME}_BUILD_SHARED
    "Add module ${_NAME} shared" ${_TARGET_SUPPORTS_SHARED_LIBS})

  # Mark these options as advanced
  mark_as_advanced(PYTHON_ENABLE_MODULE_${_NAME}
    PYTHON_MODULE_${_NAME}_BUILD_SHARED)

  if(PYTHON_ENABLE_MODULE_${_NAME})
    if(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      set(PY_MODULE_TYPE MODULE)
    else()
      set(PY_MODULE_TYPE STATIC)
      set_property(GLOBAL  APPEND  PROPERTY  PY_STATIC_MODULES_LIST ${_NAME})
    endif()

    set_property(GLOBAL  APPEND  PROPERTY  PY_MODULES_LIST ${_NAME})
    add_library(${_NAME} ${PY_MODULE_TYPE} ${ARGN})
#    target_link_libraries(${_NAME} ${PYTHON_LIBRARIES})

    if(PYTHON_MODULE_${_NAME}_BUILD_SHARED)
      set_target_properties(${_NAME} PROPERTIES PREFIX "${PYTHON_MODULE_PREFIX}")
      if(WIN32 AND NOT CYGWIN)
        set_target_properties(${_NAME} PROPERTIES SUFFIX ".pyd")
      endif()
    endif()

  endif()
endfunction()

function(PYTHON_WRITE_MODULES_HEADER _filename)

  get_property(PY_STATIC_MODULES_LIST  GLOBAL  PROPERTY PY_STATIC_MODULES_LIST)

  get_filename_component(_name "${_filename}" NAME)
  string(REPLACE "." "_" _name "${_name}")
  string(TOUPPER ${_name} _nameUpper)
  set(_filename ${CMAKE_CURRENT_BINARY_DIR}/${_filename})

  set(_filenameTmp "${_filename}.in")
  file(WRITE ${_filenameTmp} "/*Created by cmake, do not edit, changes will be lost*/\n")
  file(APPEND ${_filenameTmp}
"#ifndef ${_nameUpper}
#define ${_nameUpper}

#include <Python.h>

#ifdef __cplusplus
extern \"C\" {
#endif /* __cplusplus */

")

  foreach(_currentModule ${PY_STATIC_MODULES_LIST})
    file(APPEND ${_filenameTmp} "extern void init${PYTHON_MODULE_PREFIX}${_currentModule}(void);\n\n")
  endforeach()

  file(APPEND ${_filenameTmp}
"#ifdef __cplusplus
}
#endif /* __cplusplus */

")


  foreach(_currentModule ${PY_STATIC_MODULES_LIST})
    file(APPEND ${_filenameTmp} "int ${_name}_${_currentModule}(void) \n{\n  static char name[]=\"${PYTHON_MODULE_PREFIX}${_currentModule}\"; return PyImport_AppendInittab(name, init${PYTHON_MODULE_PREFIX}${_currentModule});\n}\n\n")
  endforeach()

  file(APPEND ${_filenameTmp} "void ${_name}_LoadAllPythonModules(void)\n{\n")
  foreach(_currentModule ${PY_STATIC_MODULES_LIST})
    file(APPEND ${_filenameTmp} "  ${_name}_${_currentModule}();\n")
  endforeach()
  file(APPEND ${_filenameTmp} "}\n\n")
  file(APPEND ${_filenameTmp} "#ifndef EXCLUDE_LOAD_ALL_FUNCTION\nvoid CMakeLoadAllPythonModules(void)\n{\n  ${_name}_LoadAllPythonModules();\n}\n#endif\n\n#endif\n")

# with configure_file() cmake complains that you may not use a file created using file(WRITE) as input file for configure_file()
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_filenameTmp}" "${_filename}" OUTPUT_QUIET ERROR_QUIET)

endfunction()
