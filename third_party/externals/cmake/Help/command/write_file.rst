write_file
----------

Deprecated. Use the file(WRITE ) command instead.

::

  write_file(filename "message to write"... [APPEND])

The first argument is the file name, the rest of the arguments are
messages to write.  If the argument APPEND is specified, then the
message will be appended.

NOTE 1: file(WRITE ...  and file(APPEND ...  do exactly the same as
this one but add some more functionality.

NOTE 2: When using write_file the produced file cannot be used as an
input to CMake (CONFIGURE_FILE, source file ...) because it will lead
to an infinite loop.  Use configure_file if you want to generate input
files to CMake.
