#=============================================================================
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

include(Platform/Darwin-Intel)
__darwin_compiler_intel(Fortran)

set(CMAKE_Fortran_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
set(CMAKE_Fortran_OSX_CURRENT_VERSION_FLAG "-current_version ")
