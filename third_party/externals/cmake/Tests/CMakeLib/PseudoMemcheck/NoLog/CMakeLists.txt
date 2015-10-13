# A dummy checker implementation that does not write the requested output file
# so it triggers an error for every checker.

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ret0.c.in" "int main(){return 0;}\n")

configure_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ret0.c.in"
  "${CMAKE_CURRENT_BINARY_DIR}/ret0.c"
  )

foreach(_pseudo IN ITEMS valgrind purify BC)
  add_executable(pseudonl_${_pseudo} "${CMAKE_CURRENT_BINARY_DIR}/ret0.c")
  set_target_properties(pseudonl_${_pseudo} PROPERTIES OUTPUT_NAME ${_pseudo})

  # Xcode 2.x forgets to create the output directory before linking
  # the individual architectures.
  if(CMAKE_OSX_ARCHITECTURES AND XCODE AND NOT "${XCODE_VERSION}" MATCHES "^[^12]")
    add_custom_command(TARGET pseudonl_${_pseudo}
                       PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CFG_INTDIR}"
    )
  endif()
endforeach()
