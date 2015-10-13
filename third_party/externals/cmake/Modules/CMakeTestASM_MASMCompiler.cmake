
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

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that the selected ASM_MASM "compiler" (should be masm or masm64)
# works. For assembler this can only check whether the compiler has been found,
# because otherwise there would have to be a separate assembler source file
# for each assembler on every architecture.

set(ASM_DIALECT "_MASM")
include(CMakeTestASMCompiler)
set(ASM_DIALECT)
