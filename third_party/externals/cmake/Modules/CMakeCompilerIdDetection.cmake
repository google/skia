
#=============================================================================
# Copyright 2014 Stephen Kelly <steveire@gmail.com>
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

function(_readFile file)
  include(${file})
  get_filename_component(name ${file} NAME_WE)
  string(REGEX REPLACE "-.*" "" CompilerId ${name})
  set(_compiler_id_version_compute_${CompilerId} ${_compiler_id_version_compute} PARENT_SCOPE)
  set(_compiler_id_simulate_${CompilerId} ${_compiler_id_simulate} PARENT_SCOPE)
  set(_compiler_id_pp_test_${CompilerId} ${_compiler_id_pp_test} PARENT_SCOPE)
endfunction()

include(${CMAKE_CURRENT_LIST_DIR}/CMakeParseArguments.cmake)

function(compiler_id_detection outvar lang)

  if (NOT lang STREQUAL Fortran)
    file(GLOB lang_files
      "${CMAKE_ROOT}/Modules/Compiler/*-DetermineCompiler.cmake")
    set(nonlang CXX)
    if (lang STREQUAL CXX)
      set(nonlang C)
    endif()

    file(GLOB nonlang_files
      "${CMAKE_ROOT}/Modules/Compiler/*-${nonlang}-DetermineCompiler.cmake")
    list(REMOVE_ITEM lang_files ${nonlang_files})
  endif()

  set(files ${lang_files})
  if (files)
    foreach(file ${files})
      _readFile(${file})
    endforeach()

    set(options ID_STRING VERSION_STRINGS ID_DEFINE PLATFORM_DEFAULT_COMPILER)
    set(oneValueArgs PREFIX)
    cmake_parse_arguments(CID "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})
    if (CID_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Unrecognized arguments: \"${CID_UNPARSED_ARGUMENTS}\"")
    endif()

    # Order is relevant here. For example, compilers which pretend to be
    # GCC must appear before the actual GCC.
    if (lang STREQUAL CXX)
      list(APPEND ordered_compilers
        Comeau
      )
    endif()
    list(APPEND ordered_compilers
      Intel
      PathScale
      Embarcadero
      Borland
      Watcom
      OpenWatcom
      SunPro
      HP
      Compaq
      zOS
      XL
      VisualAge
      PGI
      Cray
      TI
      Fujitsu
    )
    if (lang STREQUAL C)
      list(APPEND ordered_compilers
        TinyCC
      )
    endif()
    list(APPEND ordered_compilers
      SCO
      AppleClang
      Clang
      GNU
      MSVC
      ADSP
      IAR
    )
    if (lang STREQUAL C)
      list(APPEND ordered_compilers
        SDCC
      )
    endif()
    list(APPEND ordered_compilers
      MIPSpro)

    if(CID_ID_DEFINE)
      foreach(Id ${ordered_compilers})
        set(CMAKE_${lang}_COMPILER_ID_CONTENT "${CMAKE_${lang}_COMPILER_ID_CONTENT}# define ${CID_PREFIX}COMPILER_IS_${Id} 0\n")
      endforeach()
    endif()

    set(pp_if "#if")
    if (CID_VERSION_STRINGS)
      set(CMAKE_${lang}_COMPILER_ID_CONTENT "${CMAKE_${lang}_COMPILER_ID_CONTENT}\n/* Version number components: V=Version, R=Revision, P=Patch
   Version date components:   YYYY=Year, MM=Month,   DD=Day  */\n")
    endif()

    foreach(Id ${ordered_compilers})
      if (NOT _compiler_id_pp_test_${Id})
        message(FATAL_ERROR "No preprocessor test for \"${Id}\"")
      endif()
      set(id_content "${pp_if} ${_compiler_id_pp_test_${Id}}\n")
      if (CID_ID_STRING)
        set(PREFIX ${CID_PREFIX})
        string(CONFIGURE "${_compiler_id_simulate_${Id}}" SIMULATE_BLOCK @ONLY)
        set(id_content "${id_content}# define ${CID_PREFIX}COMPILER_ID \"${Id}\"${SIMULATE_BLOCK}")
      endif()
      if (CID_ID_DEFINE)
        set(id_content "${id_content}# undef ${CID_PREFIX}COMPILER_IS_${Id}\n")
        set(id_content "${id_content}# define ${CID_PREFIX}COMPILER_IS_${Id} 1\n")
      endif()
      if (CID_VERSION_STRINGS)
        set(PREFIX ${CID_PREFIX})
        set(MACRO_DEC DEC)
        set(MACRO_HEX HEX)
        string(CONFIGURE "${_compiler_id_version_compute_${Id}}" VERSION_BLOCK @ONLY)
        set(id_content "${id_content}${VERSION_BLOCK}\n")
      endif()
      set(CMAKE_${lang}_COMPILER_ID_CONTENT "${CMAKE_${lang}_COMPILER_ID_CONTENT}\n${id_content}")
      set(pp_if "#elif")
    endforeach()

    if (CID_PLATFORM_DEFAULT_COMPILER)
      set(platform_compiler_detection "
/* These compilers are either not known or too old to define an
  identification macro.  Try to identify the platform and guess that
  it is the native compiler.  */
#elif defined(__sgi)
# define ${CID_PREFIX}COMPILER_ID \"MIPSpro\"

#elif defined(__hpux) || defined(__hpua)
# define ${CID_PREFIX}COMPILER_ID \"HP\"

#else /* unknown compiler */
# define ${CID_PREFIX}COMPILER_ID \"\"")
    endif()

    set(CMAKE_${lang}_COMPILER_ID_CONTENT "${CMAKE_${lang}_COMPILER_ID_CONTENT}\n${platform_compiler_detection}\n#endif")
  endif()

  set(${outvar} ${CMAKE_${lang}_COMPILER_ID_CONTENT} PARENT_SCOPE)
endfunction()
