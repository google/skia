include(Platform/Linux-Intel)
__linux_compiler_intel(Fortran)
set(CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS} -nofor_main")
set(CMAKE_SHARED_LIBRARY_LINK_Fortran_FLAGS "")
