source_group
------------

Define a grouping for source files in IDE project generation.

.. code-block:: cmake

  source_group(<name> [FILES <src>...] [REGULAR_EXPRESSION <regex>])

Defines a group into which sources will be placed in project files.
This is intended to set up file tabs in Visual Studio.
The options are:

``FILES``
 Any source file specified explicitly will be placed in group
 ``<name>``.  Relative paths are interpreted with respect to the
 current source directory.

``REGULAR_EXPRESSION``
 Any source file whose name matches the regular expression will
 be placed in group ``<name>``.

If a source file matches multiple groups, the *last* group that
explicitly lists the file with ``FILES`` will be favored, if any.
If no group explicitly lists the file, the *last* group whose
regular expression matches the file will be favored.

The ``<name>`` of the group may contain backslashes to specify subgroups:

.. code-block:: cmake

  source_group(outer\\inner ...)

For backwards compatibility, the short-hand signature

.. code-block:: cmake

  source_group(<name> <regex>)

is equivalent to

.. code-block:: cmake

  source_group(<name> REGULAR_EXPRESSION <regex>)
