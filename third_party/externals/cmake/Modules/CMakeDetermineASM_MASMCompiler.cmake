
#=============================================================================
# Copyright 2008-2009 Kitware, Inc.
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

# Find the MS assembler (masm or masm64)

set(ASM_DIALECT "_MASM")

# if we are using the 64bit cl compiler, assume we also want the 64bit assembler
if(";${CMAKE_VS_PLATFORM_NAME};${MSVC_C_ARCHITECTURE_ID};${MSVC_CXX_ARCHITECTURE_ID};"
    MATCHES ";(Win64|Itanium|x64|IA64);")
   set(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ml64)
else()
   set(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ml)
endif()

include(CMakeDetermineASMCompiler)
set(ASM_DIALECT)
