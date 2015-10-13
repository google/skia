include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMake_BINARY_DIR}/Source
  ${CMake_SOURCE_DIR}/Source
  )

set(CMakeLib_TESTS
  testGeneratedFileStream
  testRST
  testSystemTools
  testUTF8
  testXMLParser
  testXMLSafe
  )

set(testRST_ARGS ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
  list(APPEND CMakeLib_TESTS
    testVisualStudioSlnParser
    )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testVisualStudioSlnParser.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/testVisualStudioSlnParser.h @ONLY)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testXMLParser.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testXMLParser.h @ONLY)

create_test_sourcelist(CMakeLib_TEST_SRCS CMakeLibTests.cxx ${CMakeLib_TESTS})
add_executable(CMakeLibTests ${CMakeLib_TEST_SRCS})
target_link_libraries(CMakeLibTests CMakeLib)

# Xcode 2.x forgets to create the output directory before linking
# the individual architectures.
if(CMAKE_OSX_ARCHITECTURES AND XCODE
    AND NOT "${XCODE_VERSION}" MATCHES "^[^12]")
  add_custom_command(
    TARGET CMakeLibTests
    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CFG_INTDIR}"
    )
endif()

foreach(test ${CMakeLib_TESTS})
  add_test(CMakeLib.${test} CMakeLibTests ${test} ${${test}_ARGS})
endforeach()

if(TEST_CompileCommandOutput)
  add_executable(runcompilecommands run_compile_commands.cxx)
  target_link_libraries(runcompilecommands CMakeLib)
endif()

add_subdirectory(PseudoMemcheck)
