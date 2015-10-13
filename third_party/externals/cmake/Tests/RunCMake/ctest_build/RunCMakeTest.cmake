include(RunCTest)

set(CASE_CTEST_BUILD_ARGS "")

function(run_ctest_build CASE_NAME)
  set(CASE_CTEST_BUILD_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_build(BuildQuiet QUIET)

function(run_BuildFailure)
  set(CASE_CMAKELISTS_SUFFIX_CODE [[
add_custom_target(BuildFailure ALL COMMAND command-does-not-exist)
]])
  set(CASE_TEST_PREFIX_CODE [[
cmake_policy(SET CMP0061 NEW)
]])
  set(CASE_TEST_SUFFIX_CODE [[
if (ctest_build_return_value)
  message("ctest_build returned non-zero")
else()
  message("ctest_build returned zero")
endif()
]])
  run_ctest(BuildFailure)

  if (RunCMake_GENERATOR MATCHES "Makefiles")
    set(CASE_TEST_PREFIX_CODE "")
    run_ctest(BuildFailure-CMP0061-OLD)
  endif()
endfunction()
run_BuildFailure()
