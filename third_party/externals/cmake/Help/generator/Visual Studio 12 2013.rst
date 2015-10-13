Visual Studio 12 2013
---------------------

Generates Visual Studio 12 (VS 2013) project files.

The :variable:`CMAKE_GENERATOR_PLATFORM` variable may be set
to specify a target platform name.

For compatibility with CMake versions prior to 3.1, one may specify
a target platform name optionally at the end of this generator name:

``Visual Studio 12 2013 Win64``
  Specify target platform ``x64``.

``Visual Studio 12 2013 ARM``
  Specify target platform ``ARM``.

For compatibility with CMake versions prior to 3.0, one may specify this
generator using the name "Visual Studio 12" without the year component.
