include(RunCMake)

run_cmake_command(NoArgs ${CMAKE_COMMAND})
run_cmake_command(C-no-arg ${CMAKE_COMMAND} -C)
run_cmake_command(C-no-file ${CMAKE_COMMAND} -C nosuchcachefile.txt)
run_cmake_command(cache-no-file ${CMAKE_COMMAND} nosuchsubdir/CMakeCache.txt)
run_cmake_command(lists-no-file ${CMAKE_COMMAND} nosuchsubdir/CMakeLists.txt)
run_cmake_command(D-no-arg ${CMAKE_COMMAND} -D)
run_cmake_command(U-no-arg ${CMAKE_COMMAND} -U)
run_cmake_command(E-no-arg ${CMAKE_COMMAND} -E)
run_cmake_command(E_echo_append ${CMAKE_COMMAND} -E echo_append)
run_cmake_command(E_rename-no-arg ${CMAKE_COMMAND} -E rename)
run_cmake_command(E_touch_nocreate-no-arg ${CMAKE_COMMAND} -E touch_nocreate)

run_cmake_command(E___run_iwyu-no-iwyu ${CMAKE_COMMAND} -E __run_iwyu -- command-does-not-exist)
run_cmake_command(E___run_iwyu-bad-iwyu ${CMAKE_COMMAND} -E __run_iwyu --iwyu=iwyu-does-not-exist -- command-does-not-exist)
run_cmake_command(E___run_iwyu-no--- ${CMAKE_COMMAND} -E __run_iwyu --iwyu=iwyu-does-not-exist command-does-not-exist)
run_cmake_command(E___run_iwyu-no-cc ${CMAKE_COMMAND} -E __run_iwyu --iwyu=iwyu-does-not-exist --)

run_cmake_command(G_no-arg ${CMAKE_COMMAND} -G)
run_cmake_command(G_bad-arg ${CMAKE_COMMAND} -G NoSuchGenerator)
run_cmake_command(P_no-arg ${CMAKE_COMMAND} -P)
run_cmake_command(P_no-file ${CMAKE_COMMAND} -P nosuchscriptfile.cmake)

run_cmake_command(build-no-cache
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR})
run_cmake_command(build-no-generator
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR}/cache-no-generator)
run_cmake_command(build-bad-generator
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR}/cache-bad-generator)

if(RunCMake_GENERATOR STREQUAL "Ninja")
  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Build-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

  set(RunCMake_TEST_OPTIONS -DCMAKE_VERBOSE_MAKEFILE=1)
  run_cmake(Build)
  unset(RunCMake_TEST_OPTIONS)
  run_cmake_command(Build-ninja-v ${CMAKE_COMMAND} --build .)

  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endif()

if(RunCMake_GENERATOR STREQUAL "Visual Studio 6")
  set(RunCMake_WARN_VS6 1)
  run_cmake(DeprecateVS6-WARN-ON)
  unset(RunCMake_WARN_VS6)
  run_cmake(DeprecateVS6-WARN-OFF)
elseif(RunCMake_GENERATOR STREQUAL "Visual Studio 7")
  set(RunCMake_WARN_VS70 1)
  run_cmake(DeprecateVS70-WARN-ON)
  unset(RunCMake_WARN_VS70)
  run_cmake(DeprecateVS70-WARN-OFF)
endif()

if(UNIX)
  run_cmake_command(E_create_symlink-no-arg
    ${CMAKE_COMMAND} -E create_symlink
    )
  run_cmake_command(E_create_symlink-missing-dir
    ${CMAKE_COMMAND} -E create_symlink T missing-dir/L
    )

  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR
    ${RunCMake_BINARY_DIR}/E_create_symlink-broken-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  run_cmake_command(E_create_symlink-broken-create
    ${CMAKE_COMMAND} -E create_symlink T L
    )
  run_cmake_command(E_create_symlink-broken-replace
    ${CMAKE_COMMAND} -E create_symlink . L
    )
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)

  run_cmake_command(E_create_symlink-no-replace-dir
    ${CMAKE_COMMAND} -E create_symlink T .
    )
endif()

run_cmake_command(E_env-no-command0 ${CMAKE_COMMAND} -E env)
run_cmake_command(E_env-no-command1 ${CMAKE_COMMAND} -E env TEST_ENV=1)
run_cmake_command(E_env-bad-arg1 ${CMAKE_COMMAND} -E env -bad-arg1)
run_cmake_command(E_env-set   ${CMAKE_COMMAND} -E env TEST_ENV=1 ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/E_env-set.cmake)
run_cmake_command(E_env-unset ${CMAKE_COMMAND} -E env TEST_ENV=1 ${CMAKE_COMMAND} -E env --unset=TEST_ENV ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR}/E_env-unset.cmake)

set(RunCMake_DEFAULT_stderr ".")
run_cmake_command(E_sleep-no-args ${CMAKE_COMMAND} -E sleep)
unset(RunCMake_DEFAULT_stderr)
run_cmake_command(E_sleep-bad-arg1 ${CMAKE_COMMAND} -E sleep x)
run_cmake_command(E_sleep-bad-arg2 ${CMAKE_COMMAND} -E sleep 1 -1)
run_cmake_command(E_sleep-one-tenth ${CMAKE_COMMAND} -E sleep 0.1)

run_cmake_command(P_directory ${CMAKE_COMMAND} -P ${RunCMake_SOURCE_DIR})

set(RunCMake_TEST_OPTIONS
  "-DFOO=-DBAR:BOOL=BAZ")
run_cmake(D_nested_cache)

set(RunCMake_TEST_OPTIONS
  "-DFOO:STRING=-DBAR:BOOL=BAZ")
run_cmake(D_typed_nested_cache)

set(RunCMake_TEST_OPTIONS -Wno-dev)
run_cmake(Wno-dev)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS -Wno-dev -Wdev)
run_cmake(Wdev)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS --debug-output)
run_cmake(debug-output)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS --trace)
run_cmake(trace)
unset(RunCMake_TEST_OPTIONS)

set(RunCMake_TEST_OPTIONS --debug-trycompile)
run_cmake(debug-trycompile)
unset(RunCMake_TEST_OPTIONS)

function(run_cmake_depends)
  set(RunCMake_TEST_SOURCE_DIR "${RunCMake_SOURCE_DIR}/cmake_depends")
  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/cmake_depends-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/DepTarget.dir/DependInfo.cmake" "
set(CMAKE_DEPENDS_LANGUAGES \"C\")
set(CMAKE_DEPENDS_CHECK_C
  \"${RunCMake_TEST_SOURCE_DIR}/test.c\"
  \"${RunCMake_TEST_BINARY_DIR}/CMakeFiles/DepTarget.dir/test.c.o\"
  )
")
  file(WRITE "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/CMakeDirectoryInformation.cmake" "
set(CMAKE_RELATIVE_PATH_TOP_SOURCE \"${RunCMake_TEST_SOURCE_DIR}\")
set(CMAKE_RELATIVE_PATH_TOP_BINARY \"${RunCMake_TEST_BINARY_DIR}\")
")
  run_cmake_command(cmake_depends ${CMAKE_COMMAND} -E cmake_depends
    "Unix Makefiles"
    ${RunCMake_TEST_SOURCE_DIR} ${RunCMake_TEST_SOURCE_DIR}
    ${RunCMake_TEST_BINARY_DIR} ${RunCMake_TEST_BINARY_DIR}
    ${RunCMake_TEST_BINARY_DIR}/CMakeFiles/DepTarget.dir/DependInfo.cmake
    )
endfunction()
run_cmake_depends()
