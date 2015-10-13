try_compile
-----------

.. only:: html

   .. contents::

Try building some code.

Try Compiling Whole Projects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

  try_compile(RESULT_VAR <bindir> <srcdir>
              <projectName> [<targetName>] [CMAKE_FLAGS <flags>...]
              [OUTPUT_VARIABLE <var>])

Try building a project.  The success or failure of the ``try_compile``,
i.e. ``TRUE`` or ``FALSE`` respectively, is returned in ``RESULT_VAR``.

In this form, ``<srcdir>`` should contain a complete CMake project with a
``CMakeLists.txt`` file and all sources.  The ``<bindir>`` and ``<srcdir>``
will not be deleted after this command is run.  Specify ``<targetName>`` to
build a specific target instead of the ``all`` or ``ALL_BUILD`` target.  See
below for the meaning of other options.

Try Compiling Source Files
^^^^^^^^^^^^^^^^^^^^^^^^^^

::

  try_compile(RESULT_VAR <bindir> <srcfile|SOURCES srcfile...>
              [CMAKE_FLAGS <flags>...]
              [COMPILE_DEFINITIONS <defs>...]
              [LINK_LIBRARIES <libs>...]
              [OUTPUT_VARIABLE <var>]
              [COPY_FILE <fileName> [COPY_FILE_ERROR <var>]])

Try building an executable from one or more source files.  The success or
failure of the ``try_compile``, i.e. ``TRUE`` or ``FALSE`` respectively, is
returned in ``RESULT_VAR``.

In this form the user need only supply one or more source files that include a
definition for ``main``.  CMake will create a ``CMakeLists.txt`` file to build
the source(s) as an executable that looks something like this::

  add_definitions(<expanded COMPILE_DEFINITIONS from caller>)
  include_directories(${INCLUDE_DIRECTORIES})
  link_directories(${LINK_DIRECTORIES})
  add_executable(cmTryCompileExec <srcfile>...)
  target_link_libraries(cmTryCompileExec ${LINK_LIBRARIES})

The options are:

``CMAKE_FLAGS <flags>...``
  Specify flags of the form ``-DVAR:TYPE=VALUE`` to be passed to
  the ``cmake`` command-line used to drive the test build.
  The above example shows how values for variables
  ``INCLUDE_DIRECTORIES``, ``LINK_DIRECTORIES``, and ``LINK_LIBRARIES``
  are used.

``COMPILE_DEFINITIONS <defs>...``
  Specify ``-Ddefinition`` arguments to pass to ``add_definitions``
  in the generated test project.

``COPY_FILE <fileName>``
  Copy the linked executable to the given ``<fileName>``.

``COPY_FILE_ERROR <var>``
  Use after ``COPY_FILE`` to capture into variable ``<var>`` any error
  message encountered while trying to copy the file.

``LINK_LIBRARIES <libs>...``
  Specify libraries to be linked in the generated project.
  The list of libraries may refer to system libraries and to
  :ref:`Imported Targets <Imported Targets>` from the calling project.

  If this option is specified, any ``-DLINK_LIBRARIES=...`` value
  given to the ``CMAKE_FLAGS`` option will be ignored.

``OUTPUT_VARIABLE <var>``
  Store the output from the build process the given variable.

In this version all files in ``<bindir>/CMakeFiles/CMakeTmp`` will be
cleaned automatically.  For debugging, ``--debug-trycompile`` can be
passed to ``cmake`` to avoid this clean.  However, multiple sequential
``try_compile`` operations reuse this single output directory.  If you use
``--debug-trycompile``, you can only debug one ``try_compile`` call at a time.
The recommended procedure is to protect all ``try_compile`` calls in your
project by ``if(NOT DEFINED RESULT_VAR)`` logic, configure with cmake
all the way through once, then delete the cache entry associated with
the try_compile call of interest, and then re-run cmake again with
``--debug-trycompile``.

Other Behavior Settings
^^^^^^^^^^^^^^^^^^^^^^^

Set the :variable:`CMAKE_TRY_COMPILE_CONFIGURATION` variable to choose
a build configuration.
