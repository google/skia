include(Compiler/Clang)
__compiler_clang(C)

if(NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 4.0)
  set(CMAKE_C90_STANDARD_COMPILE_OPTION "-std=c90")
  set(CMAKE_C90_EXTENSION_COMPILE_OPTION "-std=gnu90")

  set(CMAKE_C99_STANDARD_COMPILE_OPTION "-std=c99")
  set(CMAKE_C99_EXTENSION_COMPILE_OPTION "-std=gnu99")

  set(CMAKE_C11_STANDARD_COMPILE_OPTION "-std=c11")
  set(CMAKE_C11_EXTENSION_COMPILE_OPTION "-std=gnu11")
endif()

if(NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 4.0)
  set(CMAKE_C_STANDARD_DEFAULT 99)
endif()

macro(cmake_record_c_compile_features)
  macro(_get_appleclang_features std_version list)
    record_compiler_features(C "${std_version}" ${list})
  endmacro()

  set(_result 0)
  if (NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 4.0)
    _get_appleclang_features(${CMAKE_C11_STANDARD_COMPILE_OPTION} CMAKE_C11_COMPILE_FEATURES)
    if (_result EQUAL 0)
      _get_appleclang_features(${CMAKE_C99_STANDARD_COMPILE_OPTION} CMAKE_C99_COMPILE_FEATURES)
    endif()
    if (_result EQUAL 0)
      _get_appleclang_features(${CMAKE_C90_STANDARD_COMPILE_OPTION} CMAKE_C90_COMPILE_FEATURES)
    endif()
  endif()
endmacro()
