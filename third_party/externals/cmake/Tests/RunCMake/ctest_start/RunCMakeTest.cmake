include(RunCTest)

set(CASE_CTEST_START_ARGS "")

function(run_ctest_start CASE_NAME)
  set(CASE_CTEST_START_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_start(StartQuiet Experimental QUIET)
