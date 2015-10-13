MACOSX_RPATH
------------

Whether to use rpaths on Mac OS X.

When this property is set to true, the directory portion of
the "install_name" field of shared libraries will be ``@rpath``
unless overridden by :prop_tgt:`INSTALL_NAME_DIR`.  Runtime
paths will also be embedded in binaries using this target and
can be controlled by the :prop_tgt:`INSTALL_RPATH` target property.
This property is initialized by the value of the variable
:variable:`CMAKE_MACOSX_RPATH` if it is set when a target is
created.

Policy CMP0042 was introduced to change the default value of
MACOSX_RPATH to ON.  This is because use of ``@rpath`` is a
more flexible and powerful alternative to ``@executable_path`` and
``@loader_path``.
