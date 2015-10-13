CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE
--------------------------------------

Automatically add the current source- and build directories to the INTERFACE_INCLUDE_DIRECTORIES.

If this variable is enabled, CMake automatically adds for each shared
library target, static library target, module target and executable
target, ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY_DIR} to
the INTERFACE_INCLUDE_DIRECTORIES.By default
CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE is OFF.
