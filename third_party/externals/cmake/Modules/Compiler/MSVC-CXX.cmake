
if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.0)
  # MSVC has no specific language level or flags to change it.
  set(CMAKE_CXX_STANDARD_DEFAULT "")
endif()

macro(cmake_record_cxx_compile_features)
  record_compiler_features(CXX "" CMAKE_CXX_COMPILE_FEATURES)
endmacro()
