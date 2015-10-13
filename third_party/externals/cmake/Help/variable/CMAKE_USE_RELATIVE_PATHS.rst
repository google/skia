CMAKE_USE_RELATIVE_PATHS
------------------------

Use relative paths (May not work!).

If this is set to TRUE, then CMake will use relative paths between the
source and binary tree.  This option does not work for more
complicated projects, and relative paths are used when possible.  In
general, it is not possible to move CMake generated makefiles to a
different location regardless of the value of this variable.
