cmake_minimum_required(VERSION 3.0)
project(target_compile_features)

if (NOT CMAKE_CXX_COMPILE_FEATURES AND NOT CMAKE_C_COMPILE_FEATURES)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_dummy.cpp"
    "int main(int,char**) { return 0; }\n"
  )
  add_executable(target_compile_features "${CMAKE_CURRENT_BINARY_DIR}/test_dummy.cpp")
  return()
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

if (CMAKE_C_COMPILE_FEATURES)
  add_executable(target_compile_features main.c)
  target_compile_features(target_compile_features
    PRIVATE c_restrict
  )

  add_library(lib_restrict lib_restrict.c)
  target_compile_features(lib_restrict
    PUBLIC c_restrict
  )

  add_executable(restrict_user restrict_user.c)
  target_link_libraries(restrict_user lib_restrict)
endif()

if (CMAKE_CXX_COMPILE_FEATURES)
  if (CMAKE_C_COMPILE_FEATURES)
    set(target_suffix _cxx)
  endif()
  add_executable(target_compile_features${target_suffix} main.cpp)
  target_compile_features(target_compile_features${target_suffix}
    PRIVATE cxx_auto_type
  )

  add_library(lib_auto_type lib_auto_type.cpp)
  target_compile_features(lib_auto_type
    PUBLIC cxx_auto_type
  )

  add_executable(lib_user lib_user.cpp)
  target_link_libraries(lib_user lib_auto_type)
endif()
