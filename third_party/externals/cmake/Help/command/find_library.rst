find_library
------------

.. |FIND_XXX| replace:: find_library
.. |NAMES| replace:: NAMES name1 [name2 ...] [NAMES_PER_DIR]
.. |SEARCH_XXX| replace:: library
.. |SEARCH_XXX_DESC| replace:: library
.. |XXX_SUBDIR| replace:: lib

.. |CMAKE_PREFIX_PATH_XXX| replace::
   <prefix>/lib/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   |CMAKE_PREFIX_PATH_XXX_SUBDIR|
.. |CMAKE_XXX_PATH| replace:: CMAKE_LIBRARY_PATH
.. |CMAKE_XXX_MAC_PATH| replace:: CMAKE_FRAMEWORK_PATH

.. |SYSTEM_ENVIRONMENT_PATH_XXX| replace:: Directories in LIB,
   <prefix>/lib/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   |SYSTEM_ENVIRONMENT_PREFIX_PATH_XXX_SUBDIR|,
   and the directories in PATH itself.

.. |CMAKE_SYSTEM_PREFIX_PATH_XXX| replace::
   <prefix>/lib/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   |CMAKE_SYSTEM_PREFIX_PATH_XXX_SUBDIR|
.. |CMAKE_SYSTEM_XXX_PATH| replace:: CMAKE_SYSTEM_LIBRARY_PATH
.. |CMAKE_SYSTEM_XXX_MAC_PATH| replace:: CMAKE_SYSTEM_FRAMEWORK_PATH

.. |CMAKE_FIND_ROOT_PATH_MODE_XXX| replace::
   :variable:`CMAKE_FIND_ROOT_PATH_MODE_LIBRARY`

.. include:: FIND_XXX.txt

When more than one value is given to the NAMES option this command by
default will consider one name at a time and search every directory
for it.  The NAMES_PER_DIR option tells this command to consider one
directory at a time and search for all names in it.

Each library name given to the ``NAMES`` option is first considered
as a library file name and then considered with platform-specific
prefixes (e.g. ``lib``) and suffixes (e.g. ``.so``).  Therefore one
may specify library file names such as ``libfoo.a`` directly.
This can be used to locate static libraries on UNIX-like systems.

If the library found is a framework, then VAR will be set to the full
path to the framework <fullPath>/A.framework.  When a full path to a
framework is used as a library, CMake will use a -framework A, and a
-F<fullPath> to link the framework to the target.

If the global property FIND_LIBRARY_USE_LIB64_PATHS is set all search
paths will be tested as normal, with "64/" appended, and with all
matches of "lib/" replaced with "lib64/".  This property is
automatically set for the platforms that are known to need it if at
least one of the languages supported by the PROJECT command is
enabled.
