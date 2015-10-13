CMAKE_GENERATOR_PLATFORM
------------------------

Generator-specific target platform name specified by user.

Some CMake generators support a target platform name to be given
to the native build system to choose a compiler toolchain.
If the user specifies a platform name (e.g. via the cmake -A option)
the value will be available in this variable.

The value of this variable should never be modified by project code.
A toolchain file specified by the :variable:`CMAKE_TOOLCHAIN_FILE`
variable may initialize ``CMAKE_GENERATOR_PLATFORM``.  Once a given
build tree has been initialized with a particular value for this
variable, changing the value has undefined behavior.
