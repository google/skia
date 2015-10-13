
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

if(CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CPackZIP.cmake may only be used by CPack internally.")
endif()

find_program(ZIP_EXECUTABLE wzzip PATHS "$ENV{ProgramFiles}/WinZip")
if(ZIP_EXECUTABLE)
  set(CPACK_ZIP_COMMAND "\"${ZIP_EXECUTABLE}\" -P \"<ARCHIVE>\" @<FILELIST>")
  set(CPACK_ZIP_NEED_QUOTES TRUE)
endif()

if(NOT ZIP_EXECUTABLE)
  find_program(ZIP_EXECUTABLE 7z PATHS "$ENV{ProgramFiles}/7-Zip")
  if(ZIP_EXECUTABLE)
    set(CPACK_ZIP_COMMAND "\"${ZIP_EXECUTABLE}\" a -tzip \"<ARCHIVE>\" @<FILELIST>")
  set(CPACK_ZIP_NEED_QUOTES TRUE)
  endif()
endif()

if(NOT ZIP_EXECUTABLE)
  find_package(Cygwin)
  find_program(ZIP_EXECUTABLE zip PATHS "${CYGWIN_INSTALL_PATH}/bin")
  if(ZIP_EXECUTABLE)
    set(CPACK_ZIP_COMMAND "\"${ZIP_EXECUTABLE}\" -r \"<ARCHIVE>\" . -i@<FILELIST>")
    set(CPACK_ZIP_NEED_QUOTES FALSE)
  endif()
endif()

