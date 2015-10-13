find_file
---------

.. |FIND_XXX| replace:: find_file
.. |NAMES| replace:: NAMES name1 [name2 ...]
.. |SEARCH_XXX| replace:: full path to a file
.. |SEARCH_XXX_DESC| replace:: full path to named file
.. |XXX_SUBDIR| replace:: include

.. |CMAKE_PREFIX_PATH_XXX| replace::
   <prefix>/include/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   |CMAKE_PREFIX_PATH_XXX_SUBDIR|
.. |CMAKE_XXX_PATH| replace:: CMAKE_INCLUDE_PATH
.. |CMAKE_XXX_MAC_PATH| replace:: CMAKE_FRAMEWORK_PATH

.. |SYSTEM_ENVIRONMENT_PATH_XXX| replace:: Directories in INCLUDE,
   <prefix>/include/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   |SYSTEM_ENVIRONMENT_PREFIX_PATH_XXX_SUBDIR|,
   and the directories in PATH itself.

.. |CMAKE_SYSTEM_PREFIX_PATH_XXX| replace::
   <prefix>/include/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   |CMAKE_SYSTEM_PREFIX_PATH_XXX_SUBDIR|
.. |CMAKE_SYSTEM_XXX_PATH| replace:: CMAKE_SYSTEM_INCLUDE_PATH
.. |CMAKE_SYSTEM_XXX_MAC_PATH| replace:: CMAKE_SYSTEM_FRAMEWORK_PATH

.. |CMAKE_FIND_ROOT_PATH_MODE_XXX| replace::
   :variable:`CMAKE_FIND_ROOT_PATH_MODE_INCLUDE`

.. include:: FIND_XXX.txt
