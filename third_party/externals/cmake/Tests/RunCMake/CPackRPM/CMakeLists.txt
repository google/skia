cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)

set(CPACK_GENERATOR "RPM")
include(CPack)
