link_directories
----------------

Specify directories in which the linker will look for libraries.

::

  link_directories(directory1 directory2 ...)

Specify the paths in which the linker should search for libraries.
The command will apply only to targets created after it is called.
Relative paths given to this command are interpreted as relative to
the current source directory, see CMP0015.

Note that this command is rarely necessary.  Library locations
returned by find_package() and find_library() are absolute paths.
Pass these absolute library file paths directly to the
target_link_libraries() command.  CMake will ensure the linker finds
them.
