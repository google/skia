include(RunCMake)

run_cmake(CopyFileErrorNoCopyFile)
run_cmake(NoArgs)
run_cmake(OneArg)
run_cmake(TwoArgs)
run_cmake(NoCopyFile)
run_cmake(NoCopyFile2)
run_cmake(NoCopyFileError)
run_cmake(NoOutputVariable)
run_cmake(NoOutputVariable2)
run_cmake(NoSources)
run_cmake(BadLinkLibraries)
run_cmake(BadSources1)
run_cmake(BadSources2)
run_cmake(NonSourceCopyFile)
run_cmake(NonSourceCompileDefinitions)

run_cmake(CMP0056)

if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/RerunCMake-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  set(in_tc  "${RunCMake_TEST_BINARY_DIR}/TryCompileInput.c")
  file(WRITE "${in_tc}" "int main(void) { return 0; }\n")

  # Older Ninja keeps all rerun output on stdout
  set(ninja "")
  if(RunCMake_GENERATOR STREQUAL "Ninja")
    execute_process(COMMAND ${RunCMake_MAKE_PROGRAM} --version
      OUTPUT_VARIABLE ninja_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(ninja_version VERSION_LESS 1.5)
      set(ninja -ninja-no-console)
    endif()
  endif()

  message(STATUS "RerunCMake: first configuration...")
  run_cmake(RerunCMake)
  run_cmake_command(RerunCMake-nowork${ninja} ${CMAKE_COMMAND} --build .)

  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1) # handle 1s resolution
  message(STATUS "RerunCMake: modify try_compile input...")
  file(WRITE "${in_tc}" "does-not-compile\n")
  run_cmake_command(RerunCMake-rerun${ninja} ${CMAKE_COMMAND} --build .)
  run_cmake_command(RerunCMake-nowork${ninja} ${CMAKE_COMMAND} --build .)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endif()
