BUILD_WITH_INSTALL_RPATH
------------------------

Should build tree targets have install tree rpaths.

BUILD_WITH_INSTALL_RPATH is a boolean specifying whether to link the
target in the build tree with the INSTALL_RPATH.  This takes
precedence over SKIP_BUILD_RPATH and avoids the need for relinking
before installation.  This property is initialized by the value of the
variable CMAKE_BUILD_WITH_INSTALL_RPATH if it is set when a target is
created.
