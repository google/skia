
#=============================================================================
# Copyright 2010 Kitware, Inc.
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

# Find the nasm assembler. yasm (http://www.tortall.net/projects/yasm/) is nasm compatible

set(CMAKE_ASM_NASM_COMPILER_LIST nasm yasm)

if(NOT CMAKE_ASM_NASM_COMPILER)
  find_program(CMAKE_ASM_NASM_COMPILER nasm
    "$ENV{ProgramFiles}/NASM")
endif()

# Load the generic DetermineASM compiler file with the DIALECT set properly:
set(ASM_DIALECT "_NASM")
include(CMakeDetermineASMCompiler)
set(ASM_DIALECT)
