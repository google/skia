include(RunCMake)

set(RunCMake_TEST_OUTPUT_MERGE 1)
run_cmake_command(MergeOutput ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/MergeOutput.cmake)
unset(RunCMake_TEST_OUTPUT_MERGE)

run_cmake_command(MergeOutputFile ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/MergeOutputFile.cmake)
run_cmake_command(MergeOutputVars ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/MergeOutputVars.cmake)
