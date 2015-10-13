set(CMAKE_CROSSCOMPILING 1)
enable_testing()
add_test(NAME DoesNotUseEmulator
  COMMAND ${CMAKE_COMMAND} -E echo "Hi")

add_executable(generated_exe simple_src.cxx)
add_test(NAME UsesEmulator
  COMMAND generated_exe)
