include(Compiler/PGI)
__compiler_pgi(Fortran)

set(CMAKE_Fortran_FORMAT_FIXED_FLAG "-Mnofreeform")
set(CMAKE_Fortran_FORMAT_FREE_FLAG "-Mfreeform")

set(CMAKE_Fortran_FLAGS_INIT "${CMAKE_Fortran_FLAGS_INIT} -Mpreprocess -Kieee")
set(CMAKE_Fortran_FLAGS_DEBUG_INIT "${CMAKE_Fortran_FLAGS_DEBUG_INIT} -Mbounds")

set(CMAKE_Fortran_MODDIR_FLAG "-module ")
