
enable_language(CXX)

# Ensure CMake warns even if toolchain does not really have these flags.
set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden")
set(CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY "-fvisibility=")

include(CMP0063-Common.cmake)
