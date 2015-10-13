cmake_minimum_required
----------------------

Set the minimum required version of cmake for a project.

::

  cmake_minimum_required(VERSION major[.minor[.patch[.tweak]]]
                         [FATAL_ERROR])

If the current version of CMake is lower than that required it will
stop processing the project and report an error.  When a version
higher than 2.4 is specified the command implicitly invokes

::

  cmake_policy(VERSION major[.minor[.patch[.tweak]]])

which sets the cmake policy version level to the version specified.
When version 2.4 or lower is given the command implicitly invokes

::

  cmake_policy(VERSION 2.4)

which enables compatibility features for CMake 2.4 and lower.

The FATAL_ERROR option is accepted but ignored by CMake 2.6 and
higher.  It should be specified so CMake versions 2.4 and lower fail
with an error instead of just a warning.

.. note::
  Call the ``cmake_minimum_required()`` command at the beginning of
  the top-level ``CMakeLists.txt`` file even before calling the
  :command:`project` command.  It is important to establish version
  and policy settings before invoking other commands whose behavior
  they may affect.  See also policy :policy:`CMP0000`.

  Calling ``cmake_minimum_required()`` inside a :command:`function`
  limits some effects to the function scope when invoked.  Such calls
  should not be made with the intention of having global effects.
