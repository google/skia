include(RunCMake)

function(run_cpack_rpm_test TEST_NAME)
  set(RunCMake_TEST_NO_CLEAN TRUE)
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${TEST_NAME}-build")
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -D RunCMake_TEST=${TEST_NAME} "${RunCMake_SOURCE_DIR}"
    WORKING_DIRECTORY "${RunCMake_TEST_BINARY_DIR}"
    OUTPUT_QUIET
    ERROR_QUIET
    )
  run_cmake_command(${TEST_NAME} ${CMAKE_CPACK_COMMAND})
endfunction()

run_cpack_rpm_test(CPackRPM_PARTIALLY_RELOCATABLE_WARNING)
