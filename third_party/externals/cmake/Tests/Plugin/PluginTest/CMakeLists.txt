cmake_minimum_required(VERSION 2.8)

project(PluginTest)

include(${CMAKE_CURRENT_BINARY_DIR}/../mods.cmake)
get_property(configs TARGET exp_example_mod_1 PROPERTY IMPORTED_CONFIGURATIONS)
foreach(c ${configs})
  string(TOUPPER "${c}" CONFIG)
  get_property(soname1 TARGET exp_example_mod_1 PROPERTY IMPORTED_SONAME_${CONFIG})
  get_property(soname2 TARGET exp_example_mod_2 PROPERTY IMPORTED_NO_SONAME_${CONFIG})
  if(soname1)
    message(STATUS "exp_example_mod_1 has IMPORTED_SONAME_${CONFIG} as expected: ${soname1}")
  else()
    message(SEND_ERROR "exp_example_mod_1 does not have IMPORTED_SONAME_${CONFIG} but should")
  endif()
  if(soname2)
    message(STATUS "exp_example_mod_2 has IMPORTED_NO_SONAME_${CONFIG} as expected: ${soname2}")
  else()
    message(SEND_ERROR "exp_example_mod_2 does not have IMPORTED_NO_SONAME_${CONFIG} but should")
  endif()
endforeach()

# Parse the binary to check for SONAME if possible.
if("${CMAKE_EXECUTABLE_FORMAT}" MATCHES "ELF")
  find_program(READELF_EXE readelf)
  if(READELF_EXE)
    add_custom_target(check_mod_soname ALL COMMAND
      ${CMAKE_COMMAND} -Dreadelf=${READELF_EXE}
                        -Dmod1=$<TARGET_FILE:exp_example_mod_1>
                        -Dmod2=$<TARGET_FILE:exp_example_mod_2>
      -P ${CMAKE_CURRENT_SOURCE_DIR}/../check_mod_soname.cmake
      )
  endif()
endif()
