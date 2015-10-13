CPACK_PACKAGING_INSTALL_PREFIX
------------------------------

The prefix used in the built package.

Each CPack generator has a default value (like /usr).  This default
value may be overwritten from the CMakeLists.txt or the cpack command
line by setting an alternative value.

e.g.  set(CPACK_PACKAGING_INSTALL_PREFIX "/opt")

This is not the same purpose as CMAKE_INSTALL_PREFIX which is used
when installing from the build tree without building a package.
