
cmake_minimum_required(VERSION 2.8)

project(lib_shared_and_static)

include(GenerateExportHeader)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if (CMAKE_CXX_FLAGS MATCHES "-fvisibility=hidden")
  message(SEND_ERROR "Do not use add_compiler_export_flags before adding this directory")
endif()
if (CMAKE_CXX_FLAGS MATCHES "-fvisibility-inlines-hidden")
  message(SEND_ERROR "Do not use add_compiler_export_flags before adding this directory")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(lib_SRCS
  libshared_and_static.cpp
)

add_library(shared_variant SHARED ${lib_SRCS})
set_target_properties(shared_variant PROPERTIES DEFINE_SYMBOL SHARED_VARIANT_MAKEDLL)
add_library(static_variant ${lib_SRCS})

generate_export_header(shared_variant
  BASE_NAME libshared_and_static
  PREFIX_NAME MYPREFIX_
)

set_target_properties(static_variant PROPERTIES COMPILE_FLAGS -DLIBSHARED_AND_STATIC_STATIC_DEFINE)

export(TARGETS shared_variant static_variant FILE Targets.cmake)
