
#=============================================================================
# Copyright 2001-2012 Kitware, Inc.
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
if(__WINDOWS_MSVC)
  return()
endif()
set(__WINDOWS_MSVC 1)

set(CMAKE_LIBRARY_PATH_FLAG "-LIBPATH:")
set(CMAKE_LINK_LIBRARY_FLAG "")
set(MSVC 1)

# hack: if a new cmake (which uses CMAKE_LINKER) runs on an old build tree
# (where link was hardcoded) and where CMAKE_LINKER isn't in the cache
# and still cmake didn't fail in CMakeFindBinUtils.cmake (because it isn't rerun)
# hardcode CMAKE_LINKER here to link, so it behaves as it did before, Alex
if(NOT DEFINED CMAKE_LINKER)
   set(CMAKE_LINKER link)
endif()

if(CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_CL_NOLOGO)
else()
  set(CMAKE_CL_NOLOGO "/nologo")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  set(CMAKE_CREATE_WIN32_EXE "/entry:WinMainCRTStartup")
  set(CMAKE_CREATE_CONSOLE_EXE "/entry:mainACRTStartup")
  set(_PLATFORM_LINK_FLAGS " /subsystem:windowsce")
else()
  set(CMAKE_CREATE_WIN32_EXE "/subsystem:windows")
  set(CMAKE_CREATE_CONSOLE_EXE "/subsystem:console")
  set(_PLATFORM_LINK_FLAGS "")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio 6")
   set (CMAKE_NO_BUILD_TYPE 1)
endif()
if(NOT CMAKE_NO_BUILD_TYPE AND CMAKE_GENERATOR MATCHES "Visual Studio")
  set (CMAKE_NO_BUILD_TYPE 1)
endif()

if("${CMAKE_GENERATOR}" MATCHES "Visual Studio")
  set(MSVC_IDE 1)
else()
  set(MSVC_IDE 0)
endif()

if(NOT MSVC_VERSION)
  if(CMAKE_C_SIMULATE_VERSION)
    set(_compiler_version ${CMAKE_C_SIMULATE_VERSION})
  elseif(CMAKE_CXX_SIMULATE_VERSION)
    set(_compiler_version ${CMAKE_CXX_SIMULATE_VERSION})
  elseif(CMAKE_Fortran_SIMULATE_VERSION)
    set(_compiler_version ${CMAKE_Fortran_SIMULATE_VERSION})
  elseif(CMAKE_C_COMPILER_VERSION)
    set(_compiler_version ${CMAKE_C_COMPILER_VERSION})
  else()
    set(_compiler_version ${CMAKE_CXX_COMPILER_VERSION})
  endif()
  if("${_compiler_version}" MATCHES "^([0-9]+)\\.([0-9]+)")
    math(EXPR MSVC_VERSION "${CMAKE_MATCH_1}*100 + ${CMAKE_MATCH_2}")
  else()
    message(FATAL_ERROR "MSVC compiler version not detected properly: ${_compiler_version}")
  endif()

  set(MSVC10)
  set(MSVC11)
  set(MSVC12)
  set(MSVC14)
  set(MSVC60)
  set(MSVC70)
  set(MSVC71)
  set(MSVC80)
  set(MSVC90)
  set(CMAKE_COMPILER_2005)
  set(CMAKE_COMPILER_SUPPORTS_PDBTYPE)
  if(NOT "${_compiler_version}" VERSION_LESS 19)
    set(MSVC14 1)
  elseif(NOT "${_compiler_version}" VERSION_LESS 18)
    set(MSVC12 1)
  elseif(NOT "${_compiler_version}" VERSION_LESS 17)
    set(MSVC11 1)
  elseif(NOT  "${_compiler_version}" VERSION_LESS 16)
    set(MSVC10 1)
  elseif(NOT  "${_compiler_version}" VERSION_LESS 15)
    set(MSVC90 1)
  elseif(NOT  "${_compiler_version}" VERSION_LESS 14)
    set(MSVC80 1)
    set(CMAKE_COMPILER_2005 1)
  elseif(NOT  "${_compiler_version}" VERSION_LESS 13.10)
    set(MSVC71 1)
  elseif(NOT  "${_compiler_version}" VERSION_LESS 13)
    set(MSVC70 1)
  else()
    set(MSVC60 1)
    set(CMAKE_COMPILER_SUPPORTS_PDBTYPE 1)
  endif()
endif()

if(MSVC_C_ARCHITECTURE_ID MATCHES 64 OR MSVC_CXX_ARCHITECTURE_ID MATCHES 64)
  set(CMAKE_CL_64 1)
else()
  set(CMAKE_CL_64 0)
endif()
if(CMAKE_FORCE_WIN64 OR CMAKE_FORCE_IA64)
  set(CMAKE_CL_64 1)
endif()

if("${MSVC_VERSION}" GREATER 1599)
  set(MSVC_INCREMENTAL_DEFAULT ON)
endif()

# default to Debug builds
set(CMAKE_BUILD_TYPE_INIT Debug)

# Compute an architecture family from the architecture id.
foreach(lang C CXX)
  set(_MSVC_${lang}_ARCHITECTURE_FAMILY "${MSVC_${lang}_ARCHITECTURE_ID}")
  if(_MSVC_${lang}_ARCHITECTURE_FAMILY MATCHES "^ARM")
    set(_MSVC_${lang}_ARCHITECTURE_FAMILY "ARM")
  elseif(_MSVC_${lang}_ARCHITECTURE_FAMILY MATCHES "^SH")
    set(_MSVC_${lang}_ARCHITECTURE_FAMILY "SHx")
  endif()
endforeach()

if(WINCE)
  foreach(lang C CXX)
    string(TOUPPER "${_MSVC_${lang}_ARCHITECTURE_FAMILY}" _MSVC_${lang}_ARCHITECTURE_FAMILY_UPPER)
  endforeach()

  if("${CMAKE_SYSTEM_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)")
    math(EXPR _CE_VERSION "${CMAKE_MATCH_1}*100 + ${CMAKE_MATCH_2}")
  elseif("${CMAKE_SYSTEM_VERSION}" STREQUAL "")
    set(_CE_VERSION "500")
  else()
    message(FATAL_ERROR "Invalid Windows CE version: ${CMAKE_SYSTEM_VERSION}")
  endif()

  set(_PLATFORM_DEFINES "/D_WIN32_WCE=0x${_CE_VERSION} /DUNDER_CE /DWINCE")
  set(_PLATFORM_DEFINES_C " /D${_MSVC_C_ARCHITECTURE_FAMILY} /D_${_MSVC_C_ARCHITECTURE_FAMILY_UPPER}_")
  set(_PLATFORM_DEFINES_CXX " /D${_MSVC_CXX_ARCHITECTURE_FAMILY} /D_${_MSVC_CXX_ARCHITECTURE_FAMILY_UPPER}_")

  set(_RTC1 "")
  set(_FLAGS_C "")
  set(_FLAGS_CXX " /GR /EHsc")

  foreach(lang C CXX)
    if(_MSVC_${lang}_ARCHITECTURE_FAMILY STREQUAL "ARM")
      set(_PLATFORM_DEFINES_${lang} "${_PLATFORM_DEFINES_${lang}} /D${MSVC_${lang}_ARCHITECTURE_ID}")
      if(MSVC_${lang}_ARCHITECTURE_ID MATCHES "^ARMV([45])I$")
        set(_FLAGS_${lang} "${_FLAGS_${lang}} /QRarch${CMAKE_MATCH_1}T")
      endif()
    endif()
  endforeach()

  set(CMAKE_C_STANDARD_LIBRARIES_INIT "coredll.lib ole32.lib oleaut32.lib uuid.lib commctrl.lib")
  set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} /NODEFAULTLIB:libc.lib /NODEFAULTLIB:oldnames.lib")

  if (MSVC_VERSION LESS 1600)
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT} corelibc.lib")
  endif ()
elseif(WINDOWS_PHONE OR WINDOWS_STORE)
  set(_PLATFORM_DEFINES "/DWIN32")
  set(_FLAGS_C " /DUNICODE /D_UNICODE")
  set(_FLAGS_CXX " /DUNICODE /D_UNICODE /GR /EHsc")
  if(WINDOWS_PHONE)
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "WindowsPhoneCore.lib RuntimeObject.lib PhoneAppModelHost.lib")
  elseif(_MSVC_C_ARCHITECTURE_FAMILY STREQUAL "ARM" OR _MSVC_CXX_ARCHITECTURE_FAMILY STREQUAL "ARM")
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib")
  else()
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib")
  endif()
else()
  set(_PLATFORM_DEFINES "/DWIN32")

  if(_MSVC_C_ARCHITECTURE_FAMILY STREQUAL "ARM" OR _MSVC_CXX_ARCHITECTURE_FAMILY STREQUAL "ARM")
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib")
  elseif(MSVC_VERSION GREATER 1310)
    set(_RTC1 "/RTC1")
    set(_FLAGS_CXX " /GR /EHsc")
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib")
  else()
    set(_RTC1 "/GZ")
    set(_FLAGS_CXX " /GR /GX")
    set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib")
  endif()

  if(MSVC_VERSION LESS 1310)
    set(_FLAGS_C   " /Zm1000${_FLAGS_C}")
    set(_FLAGS_CXX " /Zm1000${_FLAGS_CXX}")
  endif()
endif()

set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

# executable linker flags
set (CMAKE_LINK_DEF_FILE_FLAG "/DEF:")
# set the machine type
if(MSVC_C_ARCHITECTURE_ID)
  if(MSVC_C_ARCHITECTURE_ID MATCHES "^ARMV.I")
    set(_MACHINE_ARCH_FLAG "/machine:THUMB")
  elseif(_MSVC_C_ARCHITECTURE_FAMILY STREQUAL "ARM")
    set(_MACHINE_ARCH_FLAG "/machine:ARM")
  else()
    set(_MACHINE_ARCH_FLAG "/machine:${MSVC_C_ARCHITECTURE_ID}")
  endif()
elseif(MSVC_CXX_ARCHITECTURE_ID)
  if(MSVC_CXX_ARCHITECTURE_ID MATCHES "^ARMV.I")
    set(_MACHINE_ARCH_FLAG "/machine:THUMB")
  elseif(_MSVC_CXX_ARCHITECTURE_FAMILY STREQUAL "ARM")
    set(_MACHINE_ARCH_FLAG "/machine:ARM")
  else()
    set(_MACHINE_ARCH_FLAG "/machine:${MSVC_CXX_ARCHITECTURE_ID}")
  endif()
elseif(MSVC_Fortran_ARCHITECTURE_ID)
  set(_MACHINE_ARCH_FLAG "/machine:${MSVC_Fortran_ARCHITECTURE_ID}")
endif()
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} ${_MACHINE_ARCH_FLAG}")
unset(_MACHINE_ARCH_FLAG)

# add /debug and /INCREMENTAL:YES to DEBUG and RELWITHDEBINFO also add pdbtype
# on versions that support it
set( MSVC_INCREMENTAL_YES_FLAG "")
if(NOT WINDOWS_PHONE AND NOT WINDOWS_STORE)
  if(NOT MSVC_INCREMENTAL_DEFAULT)
    set( MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL:YES")
  else()
    set(  MSVC_INCREMENTAL_YES_FLAG "/INCREMENTAL" )
  endif()
endif()

if (CMAKE_COMPILER_SUPPORTS_PDBTYPE)
  set (CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "/debug /pdbtype:sept ${MSVC_INCREMENTAL_YES_FLAG}")
  set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "/debug /pdbtype:sept ${MSVC_INCREMENTAL_YES_FLAG}")
else ()
  set (CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT "/debug ${MSVC_INCREMENTAL_YES_FLAG}")
  set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT "/debug ${MSVC_INCREMENTAL_YES_FLAG}")
endif ()
# for release and minsize release default to no incremental linking
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT "/INCREMENTAL:NO")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "/INCREMENTAL:NO")

# copy the EXE_LINKER flags to SHARED and MODULE linker flags
# shared linker flags
set (CMAKE_SHARED_LINKER_FLAGS_INIT ${CMAKE_EXE_LINKER_FLAGS_INIT})
set (CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT ${CMAKE_EXE_LINKER_FLAGS_DEBUG_INIT})
set (CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT})
set (CMAKE_SHARED_LINKER_FLAGS_RELEASE_INIT ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT})
set (CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL_INIT ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT})
# module linker flags
set (CMAKE_MODULE_LINKER_FLAGS_INIT ${CMAKE_SHARED_LINKER_FLAGS_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_DEBUG_INIT ${CMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO_INIT ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_RELEASE_INIT ${CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT})
set (CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL_INIT ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL_INIT})

macro(__windows_compiler_msvc lang)
  if(NOT MSVC_VERSION LESS 1400)
    # for 2005 make sure the manifest is put in the dll with mt
    set(_CMAKE_VS_LINK_DLL "<CMAKE_COMMAND> -E vs_link_dll ")
    set(_CMAKE_VS_LINK_EXE "<CMAKE_COMMAND> -E vs_link_exe ")
  endif()
  set(CMAKE_${lang}_CREATE_SHARED_LIBRARY
    "${_CMAKE_VS_LINK_DLL}<CMAKE_LINKER> ${CMAKE_CL_NOLOGO} <OBJECTS> ${CMAKE_START_TEMP_FILE} /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /dll /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PLATFORM_LINK_FLAGS} <LINK_FLAGS> <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")

  set(CMAKE_${lang}_CREATE_SHARED_MODULE ${CMAKE_${lang}_CREATE_SHARED_LIBRARY})
  set(CMAKE_${lang}_CREATE_STATIC_LIBRARY  "<CMAKE_LINKER> /lib ${CMAKE_CL_NOLOGO} <LINK_FLAGS> /out:<TARGET> <OBJECTS> ")

  set(CMAKE_${lang}_COMPILE_OBJECT
    "<CMAKE_${lang}_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_CL_NOLOGO}${_COMPILE_${lang}} <FLAGS> <DEFINES> /Fo<OBJECT> /Fd<TARGET_COMPILE_PDB>${_FS_${lang}} -c <SOURCE>${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_PREPROCESSED_SOURCE
    "<CMAKE_${lang}_COMPILER> > <PREPROCESSED_SOURCE> ${CMAKE_START_TEMP_FILE} ${CMAKE_CL_NOLOGO}${_COMPILE_${lang}} <FLAGS> <DEFINES> -E <SOURCE>${CMAKE_END_TEMP_FILE}")
  set(CMAKE_${lang}_CREATE_ASSEMBLY_SOURCE
    "<CMAKE_${lang}_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_CL_NOLOGO}${_COMPILE_${lang}} <FLAGS> <DEFINES> /FoNUL /FAs /Fa<ASSEMBLY_SOURCE> /c <SOURCE>${CMAKE_END_TEMP_FILE}")

  set(CMAKE_${lang}_USE_RESPONSE_FILE_FOR_OBJECTS 1)
  set(CMAKE_${lang}_LINK_EXECUTABLE
    "${_CMAKE_VS_LINK_EXE}<CMAKE_LINKER> ${CMAKE_CL_NOLOGO} <OBJECTS> ${CMAKE_START_TEMP_FILE} /out:<TARGET> /implib:<TARGET_IMPLIB> /pdb:<TARGET_PDB> /version:<TARGET_VERSION_MAJOR>.<TARGET_VERSION_MINOR>${_PLATFORM_LINK_FLAGS} <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>${CMAKE_END_TEMP_FILE}")

  set(CMAKE_${lang}_FLAGS_INIT "${_PLATFORM_DEFINES}${_PLATFORM_DEFINES_${lang}} /D_WINDOWS /W3${_FLAGS_${lang}}")
  set(CMAKE_${lang}_FLAGS_DEBUG_INIT "/D_DEBUG /MDd /Zi /Ob0 /Od ${_RTC1}")
  set(CMAKE_${lang}_FLAGS_RELEASE_INIT "/MD /O2 /Ob2 /D NDEBUG")
  set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO_INIT "/MD /Zi /O2 /Ob1 /D NDEBUG")
  set(CMAKE_${lang}_FLAGS_MINSIZEREL_INIT "/MD /O1 /Ob1 /D NDEBUG")
  set(CMAKE_${lang}_LINKER_SUPPORTS_PDB ON)

  if(NOT CMAKE_RC_COMPILER_INIT)
    set(CMAKE_RC_COMPILER_INIT rc)
  endif()
  if(NOT CMAKE_RC_FLAGS_INIT)
    set(CMAKE_RC_FLAGS_INIT "${_PLATFORM_DEFINES} ${_PLATFORM_DEFINES_${lang}}")
  endif()

  enable_language(RC)
endmacro()
