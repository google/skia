#=============================================================================
# Kitware Information Macro Library
# Copyright 2010-2011 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

set(test_defs KWIML_NAMESPACE=${KWIML})

# Tell CMake how to follow dependencies of sources in this directory.
set_property(DIRECTORY
  PROPERTY IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
  "KWIML_HEADER(%)=<${KWIML}/%>"
  )

# Suppress printf/scanf format warnings; we test if the sizes match.
foreach(lang C CXX)
  if(KWIML_LANGUAGE_${lang} AND CMAKE_${lang}_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} -Wno-format")
  endif()
endforeach()

if(KWIML_LANGUAGE_C)
  set(test_srcs test.c)
else()
  set(test_srcs test.cxx)
endif()
if(KWIML_LANGUAGE_C)
  list(APPEND test_defs KWIML_LANGUAGE_C)
  list(APPEND test_srcs
    test_ABI_C.c
    test_INT_C.c
    test_include_C.c
    )
endif()
if(KWIML_LANGUAGE_CXX)
  list(APPEND test_defs KWIML_LANGUAGE_CXX)
  list(APPEND test_srcs
    test_ABI_CXX.cxx
    test_INT_CXX.cxx
    test_include_CXX.cxx
    )
endif()

foreach(th test_ABI_endian test_INT_format)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${th}.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${th}.h @ONLY)
endforeach()
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
add_executable(${KWIML}_test ${test_srcs})
set_property(TARGET ${KWIML}_test PROPERTY COMPILE_DEFINITIONS ${test_defs})
set_property(TARGET ${KWIML}_test PROPERTY
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_test(${KWIML}.test ${CMAKE_CURRENT_BINARY_DIR}/${KWIML}_test)
set_property(TEST ${KWIML}.test PROPERTY LABELS ${KWIML_LABELS_TEST})

# Xcode 2.x forgets to create the output directory before linking
# the individual architectures.
if(CMAKE_OSX_ARCHITECTURES AND XCODE
    AND NOT "${XCODE_VERSION}" MATCHES "^[^12]")
  add_custom_command(
    TARGET ${KWIML}_test
    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CFG_INTDIR}"
    )
endif()
