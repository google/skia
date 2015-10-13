cmake_minimum_required (VERSION 3.0)
project(testf C CXX Fortran)
if(NOT DEFINED CMake_TEST_NESTED_MAKE_PROGRAM AND NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMake_TEST_NESTED_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM}")
endif()

message("CTEST_FULL_OUTPUT ")
set(CMAKE_VERBOSE_MAKEFILE 1)
message("ENV_FLAGS = $ENV{FFLAGS}")
message("CMAKE_Fortran_COMPILER_INIT = ${CMAKE_Fortran_COMPILER_INIT}")
message("CMAKE_Fortran_COMPILER_FULLPATH = ${CMAKE_Fortran_COMPILER_FULLPATH}")
message("CMAKE_Fortran_COMPILER = ${CMAKE_Fortran_COMPILER}")
message("CMAKE_Fortran_FLAGS = ${CMAKE_Fortran_FLAGS}")

set(_SHARED SHARED)
if(CMAKE_Fortran_COMPILER_ID MATCHES "^(XL|VisualAge)$")
  # We do not implement SHARED Fortran libs on AIX yet!
  # Workaround: Set LINKER_LANGUAGE to C, which uses 'xlc' and Fortran implicits.
  set(_SHARED STATIC)
elseif(CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
  # g77 2.96 does not support shared libs on Itanium because g2c is not -fPIC
  execute_process(COMMAND ${CMAKE_Fortran_COMPILER} --version
                  OUTPUT_VARIABLE output ERROR_VARIABLE output)
  if("${output}" MATCHES "Red Hat .* 2\\.96")
    set(_SHARED STATIC)
  endif()
endif()

# Pick a module .def file with the properly mangled symbol name.
set(world_def "")
if(WIN32 AND NOT CYGWIN)
  if(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
    set(world_def world_gnu.def)
  elseif(CMAKE_Fortran_COMPILER_ID MATCHES "Intel" OR
      CMAKE_GENERATOR MATCHES "Visual Studio") # Intel plugin
    set(world_def world_icl.def)
  endif()
endif()

add_library(hello STATIC hello.f)
add_library(world ${_SHARED} world.f ${world_def})
add_executable(testf testf.f)
target_link_libraries(testf hello world)

function(test_fortran_c_interface_module)
  message(STATUS "Testing FortranCInterface module")
  # test the C to Fortran interface module
  include(FortranCInterface)
  FortranCInterface_VERIFY()
  FortranCInterface_VERIFY(CXX)
  if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
    if(NOT CMAKE_Fortran_COMPILER_ID MATCHES "SunPro|MIPSpro|PathScale|Absoft")
      set(module_expected 1)
    endif()
    if(FortranCInterface_MODULE_FOUND OR module_expected)
      set(srcs foo.f)
      set(FORTRAN_FUNCTIONS test_mod:sub)
      set(MYC_DEFS TEST_MOD)
    else()
      message("${CMAKE_Fortran_COMPILER_ID} compilers do not support"
        " linking Fortran module procedures from C")
    endif()
  endif()
  list(APPEND FORTRAN_FUNCTIONS my_sub mysub)
  FortranCInterface_HEADER(foo.h
    MACRO_NAMESPACE "FC_"
    SYMBOL_NAMESPACE "F_"
    SYMBOLS ${FORTRAN_FUNCTIONS}
    )
  include_directories("${testf_BINARY_DIR}")

  # if the name mangling is not found for a F90 compiler
  # print out some diagnostic stuff for the dashboard
  if(NOT FortranCInterface_GLOBAL_FOUND OR
      (NOT FortranCInterface_MODULE_FOUND AND module_expected) )
    find_program(FortranCInterface_EXE
      NAMES FortranCInterface
      PATHS ${FortranCInterface_BINARY_DIR} ${FortranCInterface_BINARY_DIR}/Debug
      NO_DEFAULT_PATH
      )
    find_program(DUMPBIN dumpbin)
    find_program(NM nm)
    if(FortranCInterface_EXE)
      if(DEPENDS)
        execute_process(COMMAND ${DUMPBIN} /symbols "${FortranCInterface_EXE}"
                        OUTPUT_VARIABLE out)
        message("symbols in ${FortranCInterface_EXE}:\n${out}")
      endif()
      if(NM)
        execute_process(COMMAND ${NM} "${FortranCInterface_EXE}"
                        OUTPUT_VARIABLE out)
        message("symbols in ${FortranCInterface_EXE}:\n${out}")
      endif()
    endif()
  endif()
  message("Fortran = ${CMAKE_Fortran_COMPILER_ID}")
  message("C = ${CMAKE_C_COMPILER_ID}")

  add_library(myfort mysub.f ${srcs})

  add_library(myc myc.c)
  target_link_libraries(myc myfort)
  set_property(TARGET myc PROPERTY COMPILE_DEFINITIONS ${MYC_DEFS})

  add_library(mycxx mycxx.cxx)
  target_link_libraries(mycxx myc)

  add_executable(mainc mainc.c)
  target_link_libraries(mainc myc)
  add_executable(maincxx maincxx.c)
  target_link_libraries(maincxx mycxx)

  # print out some stuff to help debug on machines via cdash
  file(READ "${testf_BINARY_DIR}/foo.h" fooh)
  message("foo.h contents:\n${fooh}")
endfunction()

# if the id's match or the compilers are compatible, then
# call the test_fortran_c_interface_module function
if("${CMAKE_Fortran_COMPILER_ID}:${CMAKE_C_COMPILER_ID}" MATCHES
    "(Intel:MSVC|Absoft:GNU)"
    OR (CMAKE_Fortran_COMPILER_ID MATCHES CMAKE_C_COMPILER_ID ))
  test_fortran_c_interface_module()
else()
  message("Fortran does not match c compiler")
  message("Fortran = ${CMAKE_Fortran_COMPILER_ID}")
  message("C = ${CMAKE_C_COMPILER_ID}")
  # hack to make g77 work after CL has been enabled
  # as a languge, cmake needs language specific versions
  # of these variables....
  if(WIN32 AND CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CREATE_CONSOLE_EXE )
    set(CMAKE_LIBRARY_PATH_FLAG "-L")
    set(CMAKE_LINK_LIBRARY_FLAG "-l")
    set(CMAKE_LINK_LIBRARY_SUFFIX )
  endif()
  # gnu and sunpro do not use the same flags here...
  # however if LDFLAGS is used to set -m64 it causes odd stuf
  # with the fortran build
  if( (CMAKE_C_COMPILER_ID MATCHES "GNU")
      AND (CMAKE_Fortran_COMPILER_ID MATCHES "SunPro"))
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_Fortran_FLAGS "")
  endif()

endif()




set(TEST_MODULE_DEPENDS 0)
if(CMAKE_Fortran_COMPILER_SUPPORTS_F90)
  add_executable(test_module
    test_module_main.f90
    test_module_implementation.f90
    test_module_interface.f90)

  add_executable(test_use_in_comment_fixedform
    test_use_in_comment_fixedform.f)
  set_property(SOURCE test_use_in_comment_fixedform.f PROPERTY Fortran_FORMAT FIXED)
  add_executable(test_use_in_comment_freeform
    test_use_in_comment_freeform.f90)
  set_property(SOURCE test_use_in_comment_freeform.f90 PROPERTY Fortran_FORMAT FREE)

  add_executable(test_in_interface
    in_interface/main.f90
    in_interface/module.f90)

  add_definitions(-DFOO -DBAR=1)
  include_directories(${testf_SOURCE_DIR}/include)
  add_executable(test_preprocess test_preprocess.F90 test_preprocess_module.F90)

  set(TEST_MODULE_DEPENDS 1)
endif()

if(TEST_MODULE_DEPENDS)
  # Build the external project separately using a custom target.
  # Make sure it uses the same build configuration as this test.
  if(CMAKE_CONFIGURATION_TYPES)
    set(External_CONFIG_TYPE -C "${CMAKE_CFG_INTDIR}")
    set(External_BUILD_TYPE)
  else()
    set(External_CONFIG_TYPE)
    set(External_BUILD_TYPE -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
  endif()
  set(External_SOURCE_DIR "${testf_SOURCE_DIR}/External")
  set(External_BINARY_DIR "${testf_BINARY_DIR}/External")
  if("${testf_BINARY_DIR}" MATCHES " ")
    # Our build tree has a space, so the build tool supports spaces.
    # Test using modules from a path with spaces.
    set(External_BINARY_DIR "${External_BINARY_DIR} Build")
  endif()
  add_custom_command(
    OUTPUT ${testf_BINARY_DIR}/ExternalProject
    COMMAND ${CMAKE_CTEST_COMMAND}
    ARGS ${External_CONFIG_TYPE}
         --build-and-test
         ${External_SOURCE_DIR}
         ${External_BINARY_DIR}
         --build-noclean
         --build-two-config
         --build-project ExtFort
         --build-generator ${CMAKE_GENERATOR}
         --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
         --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
         --build-options -DCMAKE_Fortran_COMPILER:STRING=${CMAKE_Fortran_COMPILER}
                         -DCMAKE_Fortran_FLAGS:STRING=${CMAKE_Fortran_FLAGS}
                         -DCMAKE_Fortran_FLAGS_DEBUG:STRING=${CMAKE_Fortran_FLAGS_DEBUG}
                         -DCMAKE_Fortran_FLAGS_RELEASE:STRING=${CMAKE_Fortran_FLAGS_RELEASE}
                         -DCMAKE_Fortran_FLAGS_MINSIZEREL:STRING=${CMAKE_Fortran_FLAGS_MINSIZEREL}
                         -DCMAKE_Fortran_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_Fortran_FLAGS_RELWITHDEBINFO}
                         -DCMAKE_MAKE_PROGRAM:FILEPATH=${CMake_TEST_NESTED_MAKE_PROGRAM}
                         ${External_BUILD_TYPE}
    VERBATIM
    )
  add_custom_target(ExternalTarget ALL DEPENDS ${testf_BINARY_DIR}/ExternalProject)

  # Test module output directory if available.
  if(CMAKE_Fortran_MODDIR_FLAG)
    set(Library_MODDIR "${testf_BINARY_DIR}/Library/modules")
  else()
    set(Library_MODDIR "${testf_BINARY_DIR}/Library")
  endif()

  add_subdirectory(Library)
  add_subdirectory(Executable)
endif()
