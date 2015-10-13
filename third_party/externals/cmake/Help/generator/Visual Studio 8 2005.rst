Visual Studio 8 2005
--------------------

Generates Visual Studio 8 2005 project files.

The :variable:`CMAKE_GENERATOR_PLATFORM` variable may be set
to specify a target platform name.

For compatibility with CMake versions prior to 3.1, one may specify
a target platform name optionally at the end of this generator name:

``Visual Studio 8 2005 Win64``
  Specify target platform ``x64``.

``Visual Studio 8 2005 <WinCE-SDK>``
  Specify target platform matching a Windows CE SDK name.
