C_EXTENSIONS
------------

Boolean specifying whether compiler specific extensions are requested.

This property specifies whether compiler specific extensions should be
used.  For some compilers, this results in adding a flag such
as ``-std=gnu11`` instead of ``-std=c11`` to the compile line.  This
property is ``ON`` by default.

See the :manual:`cmake-compile-features(7)` manual for information on
compile features and a list of supported compilers.

This property is initialized by the value of
the :variable:`CMAKE_C_EXTENSIONS` variable if it is set when a target
is created.
