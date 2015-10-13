SOVERSION
---------

What version number is this target.

For shared libraries VERSION and SOVERSION can be used to specify the
build version and API version respectively.  When building or
installing appropriate symlinks are created if the platform supports
symlinks and the linker supports so-names.  If only one of both is
specified the missing is assumed to have the same version number.
SOVERSION is ignored if NO_SONAME property is set.  For shared
libraries and executables on Windows the VERSION attribute is parsed
to extract a "major.minor" version number.  These numbers are used as
the image version of the binary.
