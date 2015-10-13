include(CMakePushCheckState)

set(CMAKE_REQUIRED_DEFINITIONS defs1 )

cmake_push_check_state()

set(CMAKE_REQUIRED_DEFINITIONS defs2)

cmake_push_check_state()

set(CMAKE_REQUIRED_DEFINITIONS defs3)

cmake_pop_check_state()

if (NOT "${CMAKE_REQUIRED_DEFINITIONS}" STREQUAL "defs2")
  set(fatal TRUE)
  message("ERROR: "CMAKE_REQUIRED_DEFINITIONS is \"${CMAKE_REQUIRED_DEFINITIONS}\" (expected \"defs2\")" )
endif()

cmake_pop_check_state()

if (NOT "${CMAKE_REQUIRED_DEFINITIONS}" STREQUAL "defs1")
  set(fatal TRUE)
  message("ERROR: "CMAKE_REQUIRED_DEFINITIONS is \"${CMAKE_REQUIRED_DEFINITIONS}\" (expected \"defs1\")" )
endif()


if(fatal)
  message(FATAL_ERROR "cmake_push_check_state() test failed")
endif()
