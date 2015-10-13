CodeBlocks
----------

Generates CodeBlocks project files.

Project files for CodeBlocks will be created in the top directory and
in every subdirectory which features a CMakeLists.txt file containing
a PROJECT() call.  Additionally a hierarchy of makefiles is generated
into the build tree.  The appropriate make program can build the
project through the default make target.  A "make install" target is
also provided.

This "extra" generator may be specified as:

``CodeBlocks - MinGW Makefiles``
 Generate with :generator:`MinGW Makefiles`.

``CodeBlocks - NMake Makefiles``
 Generate with :generator:`NMake Makefiles`.

``CodeBlocks - Ninja``
 Generate with :generator:`Ninja`.

``CodeBlocks - Unix Makefiles``
 Generate with :generator:`Unix Makefiles`.
