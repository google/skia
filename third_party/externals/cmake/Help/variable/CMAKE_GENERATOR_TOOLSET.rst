CMAKE_GENERATOR_TOOLSET
-----------------------

Native build system toolset name specified by user.

Some CMake generators support a toolset name to be given to the native
build system to choose a compiler.  If the user specifies a toolset
name (e.g.  via the cmake -T option) the value will be available in
this variable.

The value of this variable should never be modified by project code.
A toolchain file specified by the :variable:`CMAKE_TOOLCHAIN_FILE`
variable may initialize ``CMAKE_GENERATOR_TOOLSET``.  Once a given
build tree has been initialized with a particular value for this
variable, changing the value has undefined behavior.
