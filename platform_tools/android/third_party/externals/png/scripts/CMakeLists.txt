
project(PNG)

# Copyright (C) 2007 Glenn Randers-Pehrson

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

set(PNGLIB_MAJOR 1)
set(PNGLIB_MINOR 2)
set(PNGLIB_RELEASE 38)
set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

# needed packages
find_package(ZLIB REQUIRED)
if(NOT WIN32)
 find_library(M_LIBRARY
     NAMES m
     PATHS /usr/lib /usr/local/lib
 )
 if(NOT M_LIBRARY)
   message(STATUS
     "math library 'libm' not found - floating point support disabled")
 endif(NOT M_LIBRARY)
else(NOT WIN32)
 # not needed on windows
 set(M_LIBRARY "")
endif(NOT WIN32)


# COMMAND LINE OPTIONS
option(PNG_SHARED "Build shared lib" YES)
option(PNG_STATIC "Build static lib" YES)
if(MINGW)
  option(PNG_TESTS  "Build pngtest" NO)
else(MINGW)
  option(PNG_TESTS  "Build pngtest" YES)
endif(MINGW)
option(PNG_NO_CONSOLE_IO "FIXME" YES)
option(PNG_NO_STDIO      "FIXME" YES)
option(PNG_DEBUG         "Build with debug output" YES)
option(PNGARG            "FIXME" YES)
#TODO:
# PNG_CONSOLE_IO_SUPPORTED

# maybe needs improving, but currently I don't know when we can enable what :)
set(png_asm_tmp "OFF")
if(NOT WIN32)
 find_program(uname_executable NAMES uname PATHS /bin /usr/bin /usr/local/bin)
 if(uname_executable)
   EXEC_PROGRAM(${uname_executable} ARGS --machine OUTPUT_VARIABLE uname_output)
   if("uname_output" MATCHES "^.*i[1-9]86.*$")
      set(png_asm_tmp "ON")
   else("uname_output" MATCHES "^.*i[1-9]86.*$")
      set(png_asm_tmp "OFF")
   endif("uname_output" MATCHES "^.*i[1-9]86.*$")
 endif(uname_executable)
else(NOT WIN32)
 # this env var is normally only set on win64
 SET(TEXT "ProgramFiles(x86)")
 if("$ENV{${TEXT}}" STREQUAL "")
  set(png_asm_tmp "ON")
 endif("$ENV{${TEXT}}" STREQUAL "")
endif(NOT WIN32)

# SET LIBNAME
# msvc does not append 'lib' - do it here to have consistent name
if(MSVC)
 set(PNG_LIB_NAME lib)
endif(MSVC)
set(PNG_LIB_NAME ${PNG_LIB_NAME}png${PNGLIB_MAJOR}${PNGLIB_MINOR})

# to distinguish between debug and release lib
set(CMAKE_DEBUG_POSTFIX "d")


# OUR SOURCES
set(libpng_sources
 png.h
 pngconf.h
 png.c
 pngerror.c
 pngget.c
 pngmem.c
 pngpread.c
 pngread.c
 pngrio.c
 pngrtran.c
 pngrutil.c
 pngset.c
 pngtrans.c
 pngwio.c
 pngwrite.c
 pngwtran.c
 pngwutil.c
)
set(pngtest_sources
       pngtest.c
)
# SOME NEEDED DEFINITIONS
if(MSVC)
  add_definitions(-DPNG_NO_MODULEDEF -D_CRT_SECURE_NO_DEPRECATE)
endif(MSVC)

add_definitions(-DZLIB_DLL)

add_definitions(-DLIBPNG_NO_MMX)
add_definitions(-DPNG_NO_MMX_CODE)

if(PNG_CONSOLE_IO_SUPPORTED)
 add_definitions(-DPNG_CONSOLE_IO_SUPPORTED)
endif(PNG_CONSOLE_IO_SUPPORTED)

if(PNG_NO_CONSOLE_IO)
 add_definitions(-DPNG_NO_CONSOLE_IO)
endif(PNG_NO_CONSOLE_IO)

if(PNG_NO_STDIO)
 add_definitions(-DPNG_NO_STDIO)
endif(PNG_NO_STDIO)

if(PNG_DEBUG)
 add_definitions(-DPNG_DEBUG)
endif(PNG_DEBUG)

if(NOT M_LIBRARY AND NOT WIN32)
 add_definitions(-DPNG_NO_FLOATING_POINT_SUPPORTED)
endif(NOT M_LIBRARY AND NOT WIN32)

# NOW BUILD OUR TARGET
include_directories(${PNG_SOURCE_DIR} ${ZLIB_INCLUDE_DIR})

if(PNG_SHARED)
 add_library(${PNG_LIB_NAME} SHARED ${libpng_sources})
 target_link_libraries(${PNG_LIB_NAME} ${ZLIB_LIBRARY} ${M_LIBRARY})
endif(PNG_SHARED)
if(PNG_STATIC)
# does not work without changing name
 set(PNG_LIB_NAME_STATIC ${PNG_LIB_NAME}_static)
 add_library(${PNG_LIB_NAME_STATIC} STATIC ${libpng_sources})
endif(PNG_STATIC)

if(PNG_SHARED AND WIN32)
 set_target_properties(${PNG_LIB_NAME} PROPERTIES DEFINE_SYMBOL PNG_BUILD_DLL)
endif(PNG_SHARED AND WIN32)

if(PNG_TESTS)
# does not work with msvc due to png_lib_ver issue
 add_executable(pngtest ${pngtest_sources})
 target_link_libraries(pngtest ${PNG_LIB_NAME})
#  add_test(pngtest ${PNG_SOURCE_DIR}/pngtest.png)
endif(PNG_TESTS)


# CREATE PKGCONFIG FILES
# we use the same files like ./configure, so we have to set its vars
set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir      ${CMAKE_INSTALL_PREFIX}/lib)
set(includedir  ${CMAKE_INSTALL_PREFIX}/include)

configure_file(${PNG_SOURCE_DIR}/scripts/libpng.pc.in
  ${PNG_BINARY_DIR}/libpng.pc)
configure_file(${PNG_SOURCE_DIR}/scripts/libpng-config.in
  ${PNG_BINARY_DIR}/libpng-config)
configure_file(${PNG_SOURCE_DIR}/scripts/libpng.pc.in
  ${PNG_BINARY_DIR}/${PNGLIB_NAME}.pc)
configure_file(${PNG_SOURCE_DIR}/scripts/libpng-config.in
  ${PNG_BINARY_DIR}/${PNGLIB_NAME}-config)

# SET UP LINKS
set_target_properties(${PNG_LIB_NAME} PROPERTIES
#    VERSION 0.${PNGLIB_RELEASE}.1.2.38
     VERSION 0.${PNGLIB_RELEASE}.0
     SOVERSION 0
     CLEAN_DIRECT_OUTPUT 1)
if(NOT WIN32)
  # that's uncool on win32 - it overwrites our static import lib...
  set_target_properties(${PNG_LIB_NAME_STATIC} PROPERTIES
     OUTPUT_NAME ${PNG_LIB_NAME}
     CLEAN_DIRECT_OUTPUT 1)
endif(NOT WIN32)
# INSTALL
install_targets(/lib ${PNG_LIB_NAME})
if(PNG_STATIC)
  install_targets(/lib ${PNG_LIB_NAME_STATIC})
endif(PNG_STATIC)
install(FILES png.h pngconf.h         DESTINATION include)
install(FILES png.h pngconf.h         DESTINATION include/${PNGLIB_NAME})
install(FILES libpng.3 libpngpf.3             DESTINATION man/man3)
install(FILES png.5                           DESTINATION man/man5)
install(FILES ${PNG_BINARY_DIR}/libpng.pc     DESTINATION lib/pkgconfig)
install(FILES ${PNG_BINARY_DIR}/libpng-config      DESTINATION bin)
install(FILES ${PNG_BINARY_DIR}/${PNGLIB_NAME}.pc  DESTINATION lib/pkgconfig)
install(FILES ${PNG_BINARY_DIR}/${PNGLIB_NAME}-config DESTINATION bin)

# what's with libpng.txt and all the extra files?


# UNINSTALL
# do we need this?


# DIST
# do we need this?

# to create msvc import lib for mingw compiled shared lib
# pexports libpng.dll > libpng.def
# lib /def:libpng.def /machine:x86

