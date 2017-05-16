remove_definitions
------------------

Removes -D define flags added by add_definitions.

::

  remove_definitions(-DFOO -DBAR ...)

Removes flags (added by add_definitions) from the compiler command
line for sources in the current directory and below.
