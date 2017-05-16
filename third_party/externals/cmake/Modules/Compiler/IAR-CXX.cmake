# This file is processed when the IAR compiler is used for a C++ file

include(Compiler/IAR)

set(CMAKE_CXX_COMPILE_OBJECT  "<CMAKE_CXX_COMPILER> <SOURCE> <DEFINES> <FLAGS> -o <OBJECT>")

set(CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_CXX_COMPILER> <SOURCE> <DEFINES> <FLAGS> --preprocess=cnl <PREPROCESSED_SOURCE>")
set(CMAKE_CXX_CREATE_ASSEMBLY_SOURCE     "<CMAKE_CXX_COMPILER> <SOURCE> <DEFINES> <FLAGS> -lAH <ASSEMBLY_SOURCE> -o <OBJECT>.dummy")



if("${IAR_TARGET_ARCHITECTURE}" STREQUAL "ARM")

  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
  set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> <TARGET> --create <LINK_FLAGS> <OBJECTS> ")

endif()


if("${IAR_TARGET_ARCHITECTURE}" STREQUAL "AVR")
  set(CMAKE_CXX_OUTPUT_EXTENSION ".r90")
  if(NOT CMAKE_CXX_LINK_FLAGS)
    set(CMAKE_CXX_LINK_FLAGS "-Fmotorola")
  endif()

  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES> -o <TARGET>")
  set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> -o <TARGET> <OBJECTS> ")

endif()

# add the target specific include directory:
get_filename_component(_compilerDir "${CMAKE_C_COMPILER}" PATH)
get_filename_component(_compilerDir "${_compilerDir}" PATH)
include_directories("${_compilerDir}/inc")
