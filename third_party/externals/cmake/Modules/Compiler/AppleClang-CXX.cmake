include(Compiler/Clang)
__compiler_clang(CXX)

if(NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
  set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden")
endif()

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.0)
  set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "-std=c++98")
  set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "-std=gnu++98")

  set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "-std=c++11")
  set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION "-std=gnu++11")
endif()

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
  # AppleClang 5.0 knows this flag, but does not set a __cplusplus macro greater than 201103L
  set(CMAKE_CXX14_STANDARD_COMPILE_OPTION "-std=c++1y")
  set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION "-std=gnu++1y")
endif()

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.0)
  set(CMAKE_CXX_STANDARD_DEFAULT 98)
endif()

macro(cmake_record_cxx_compile_features)
  macro(_get_appleclang_features std_version list)
    record_compiler_features(CXX "${std_version}" ${list})
  endmacro()

  set(_result 0)
  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.0)
    set(_result 0)
    if(CMAKE_CXX14_STANDARD_COMPILE_OPTION)
      _get_appleclang_features(${CMAKE_CXX14_STANDARD_COMPILE_OPTION} CMAKE_CXX14_COMPILE_FEATURES)
    endif()
    if (_result EQUAL 0)
      _get_appleclang_features(${CMAKE_CXX11_STANDARD_COMPILE_OPTION} CMAKE_CXX11_COMPILE_FEATURES)
    endif()
    if (_result EQUAL 0)
      _get_appleclang_features(${CMAKE_CXX98_STANDARD_COMPILE_OPTION} CMAKE_CXX98_COMPILE_FEATURES)
    endif()
  endif()
endmacro()
