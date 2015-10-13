#=============================================================================
# Copyright 2009 Kitware, Inc.
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

include(Platform/Darwin-GNU)
__darwin_compiler_gnu(Fortran)
cmake_gnu_set_sysroot_flag(Fortran)
cmake_gnu_set_osx_deployment_target_flag(Fortran)

set(CMAKE_Fortran_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
set(CMAKE_Fortran_OSX_CURRENT_VERSION_FLAG "-current_version ")
