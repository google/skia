file(GLOB INSTALLED_CURSES /usr/bin/cygncurses-*.dll)
set(MAX 0)
foreach(f ${INSTALLED_CURSES})
  if(NOT "${f}" MATCHES "\\+")
    string(REGEX REPLACE ".*-([0-9]*).dll" "\\1" NUMBER "${f}")
    if(NUMBER GREATER MAX)
      set(MAX ${NUMBER})
    endif()
  endif()
endforeach()
string(REGEX REPLACE "/usr/bin/" "\\1" NUMBER "${f}")
set(CMAKE_NCURSES_VERSION "libncurses${MAX}")
message(STATUS "Using curses version: libncurses${MAX}")
configure_file("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/cygwin-setup.hint.in"
  "${CMake_BINARY_DIR}/setup.hint")
configure_file("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/README.cygwin.in"
  "${CMake_BINARY_DIR}/Docs/${CPACK_PACKAGE_FILE_NAME}-${CPACK_CYGWIN_PATCH_NUMBER}.README")
install_files(/share/doc/Cygwin FILES
  ${CMake_BINARY_DIR}/Docs/${CPACK_PACKAGE_FILE_NAME}-${CPACK_CYGWIN_PATCH_NUMBER}.README
  )
configure_file("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/cygwin-package.sh.in"
  ${CPACK_CYGWIN_BUILD_SCRIPT})
configure_file("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/cygwin-patch.diff.in"
  ${CPACK_CYGWIN_PATCH_FILE})

