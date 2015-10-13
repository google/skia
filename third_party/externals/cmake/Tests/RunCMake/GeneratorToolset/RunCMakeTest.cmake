include(RunCMake)

set(RunCMake_GENERATOR_TOOLSET "")
run_cmake(NoToolset)

if("${RunCMake_GENERATOR}" MATCHES "Visual Studio 1[0124]|Xcode" AND NOT XCODE_BELOW_3)
  set(RunCMake_GENERATOR_TOOLSET "Test Toolset")
  run_cmake(TestToolset)
else()
  set(RunCMake_GENERATOR_TOOLSET "Bad Toolset")
  run_cmake(BadToolset)
endif()

set(RunCMake_GENERATOR_TOOLSET "")

set(RunCMake_TEST_OPTIONS -T "Extra Toolset")
run_cmake(TwoToolsets)
unset(RunCMake_TEST_OPTIONS)

if("${RunCMake_GENERATOR}" MATCHES "Visual Studio 1[0124]|Xcode" AND NOT XCODE_BELOW_3)
  set(RunCMake_TEST_OPTIONS -DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/TestToolset-toolchain.cmake)
  run_cmake(TestToolsetToolchain)
  unset(RunCMake_TEST_OPTIONS)
else()
  set(RunCMake_TEST_OPTIONS -DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/BadToolset-toolchain.cmake)
  run_cmake(BadToolsetToolchain)
  unset(RunCMake_TEST_OPTIONS)
endif()
