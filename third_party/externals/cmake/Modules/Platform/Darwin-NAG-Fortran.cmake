#=============================================================================
# Copyright 2010 Kitware, Inc.
# Copyright 2013 OpenGamma Ltd. <graham@opengamma.com>
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

set(CMAKE_Fortran_VERBOSE_FLAG "-Wl,-v") # Runs gcc under the hood.

# Need -fpp explicitly on case-insensitive filesystem.
set(CMAKE_Fortran_COMPILE_OBJECT
  "<CMAKE_Fortran_COMPILER> -fpp -o <OBJECT> <DEFINES> <FLAGS> -c <SOURCE>")

set(CMAKE_Fortran_OSX_COMPATIBILITY_VERSION_FLAG "-Wl,-compatibility_version -Wl,")
set(CMAKE_Fortran_OSX_CURRENT_VERSION_FLAG "-Wl,-current_version -Wl,")
set(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS "-Wl,-shared")
set(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-Wl,-install_name -Wl,")
set(CMAKE_Fortran_CREATE_SHARED_LIBRARY
  "<CMAKE_Fortran_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG><TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
