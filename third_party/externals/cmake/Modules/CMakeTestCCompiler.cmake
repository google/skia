
#=============================================================================
# Copyright 2003-2012 Kitware, Inc.
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

if(CMAKE_C_COMPILER_FORCED)
  # The compiler configuration was forced by the user.
  # Assume the user has configured all compiler information.
  set(CMAKE_C_COMPILER_WORKS TRUE)
  return()
endif()

include(CMakeTestCompilerCommon)

# Remove any cached result from an older CMake version.
# We now store this in CMakeCCompiler.cmake.
unset(CMAKE_C_COMPILER_WORKS CACHE)

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected C compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
if(NOT CMAKE_C_COMPILER_WORKS)
  PrintTestCompilerStatus("C" "")
  file(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCCompiler.c
    "#ifdef __cplusplus\n"
    "# error \"The CMAKE_C_COMPILER is set to a C++ compiler\"\n"
    "#endif\n"
    "#if defined(__CLASSIC_C__)\n"
    "int main(argc, argv)\n"
    "  int argc;\n"
    "  char* argv[];\n"
    "#else\n"
    "int main(int argc, char* argv[])\n"
    "#endif\n"
    "{ (void)argv; return argc-1;}\n")
  try_compile(CMAKE_C_COMPILER_WORKS ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testCCompiler.c
    OUTPUT_VARIABLE __CMAKE_C_COMPILER_OUTPUT)
  # Move result from cache to normal variable.
  set(CMAKE_C_COMPILER_WORKS ${CMAKE_C_COMPILER_WORKS})
  unset(CMAKE_C_COMPILER_WORKS CACHE)
  set(C_TEST_WAS_RUN 1)
endif()

if(NOT CMAKE_C_COMPILER_WORKS)
  PrintTestCompilerStatus("C" " -- broken")
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the C compiler works failed with "
    "the following output:\n${__CMAKE_C_COMPILER_OUTPUT}\n\n")
  message(FATAL_ERROR "The C compiler \"${CMAKE_C_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${__CMAKE_C_COMPILER_OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
else()
  if(C_TEST_WAS_RUN)
    PrintTestCompilerStatus("C" " -- works")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the C compiler works passed with "
      "the following output:\n${__CMAKE_C_COMPILER_OUTPUT}\n\n")
  endif()

  # Try to identify the ABI and configure it into CMakeCCompiler.cmake
  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerABI.cmake)
  CMAKE_DETERMINE_COMPILER_ABI(C ${CMAKE_ROOT}/Modules/CMakeCCompilerABI.c)
  # Try to identify the compiler features
  include(${CMAKE_ROOT}/Modules/CMakeDetermineCompileFeatures.cmake)
  CMAKE_DETERMINE_COMPILE_FEATURES(C)

  # Re-configure to save learned information.
  configure_file(
    ${CMAKE_ROOT}/Modules/CMakeCCompiler.cmake.in
    ${CMAKE_PLATFORM_INFO_DIR}/CMakeCCompiler.cmake
    @ONLY
    )
  include(${CMAKE_PLATFORM_INFO_DIR}/CMakeCCompiler.cmake)

  if(CMAKE_C_SIZEOF_DATA_PTR)
    foreach(f ${CMAKE_C_ABI_FILES})
      include(${f})
    endforeach()
    unset(CMAKE_C_ABI_FILES)
  endif()
endif()

unset(__CMAKE_C_COMPILER_OUTPUT)
