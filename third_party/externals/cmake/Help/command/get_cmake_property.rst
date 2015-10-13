get_cmake_property
------------------

Get a property of the CMake instance.

::

  get_cmake_property(VAR property)

Get a property from the CMake instance.  The value of the property is
stored in the variable VAR.  If the property is not found, VAR will be
set to "NOTFOUND".  Some supported properties include: VARIABLES,
CACHE_VARIABLES, COMMANDS, MACROS, and COMPONENTS.

See also the more general get_property() command.
