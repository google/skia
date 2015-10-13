CMAKE_CURRENT_BINARY_DIR
------------------------

The path to the binary directory currently being processed.

This the full path to the build directory that is currently being
processed by cmake.  Each directory added by add_subdirectory will
create a binary directory in the build tree, and as it is being
processed this variable will be set.  For in-source builds this is the
current source directory being processed.
