add_custom_command
------------------

Add a custom build rule to the generated build system.

There are two main signatures for ``add_custom_command``.

Generating Files
^^^^^^^^^^^^^^^^

The first signature is for adding a custom command to produce an output::

  add_custom_command(OUTPUT output1 [output2 ...]
                     COMMAND command1 [ARGS] [args1...]
                     [COMMAND command2 [ARGS] [args2...] ...]
                     [MAIN_DEPENDENCY depend]
                     [DEPENDS [depends...]]
                     [BYPRODUCTS [files...]]
                     [IMPLICIT_DEPENDS <lang1> depend1
                                      [<lang2> depend2] ...]
                     [WORKING_DIRECTORY dir]
                     [COMMENT comment]
                     [VERBATIM] [APPEND] [USES_TERMINAL])

This defines a command to generate specified ``OUTPUT`` file(s).
A target created in the same directory (``CMakeLists.txt`` file)
that specifies any output of the custom command as a source file
is given a rule to generate the file using the command at build time.
Do not list the output in more than one independent target that
may build in parallel or the two instances of the rule may conflict
(instead use the :command:`add_custom_target` command to drive the
command and make the other targets depend on that one).
In makefile terms this creates a new target in the following form::

  OUTPUT: MAIN_DEPENDENCY DEPENDS
          COMMAND

The options are:

``APPEND``
  Append the ``COMMAND`` and ``DEPENDS`` option values to the custom
  command for the first output specified.  There must have already
  been a previous call to this command with the same output.
  The ``COMMENT``, ``MAIN_DEPENDENCY``, and ``WORKING_DIRECTORY``
  options are currently ignored when APPEND is given, but may be
  used in the future.

``BYPRODUCTS``
  Specify the files the command is expected to produce but whose
  modification time may or may not be newer than the dependencies.
  If a byproduct name is a relative path it will be interpreted
  relative to the build tree directory corresponding to the
  current source directory.
  Each byproduct file will be marked with the :prop_sf:`GENERATED`
  source file property automatically.

  Explicit specification of byproducts is supported by the
  :generator:`Ninja` generator to tell the ``ninja`` build tool
  how to regenerate byproducts when they are missing.  It is
  also useful when other build rules (e.g. custom commands)
  depend on the byproducts.  Ninja requires a build rule for any
  generated file on which another rule depends even if there are
  order-only dependencies to ensure the byproducts will be
  available before their dependents build.

  The ``BYPRODUCTS`` option is ignored on non-Ninja generators
  except to mark byproducts ``GENERATED``.

``COMMAND``
  Specify the command-line(s) to execute at build time.
  If more than one ``COMMAND`` is specified they will be executed in order,
  but *not* necessarily composed into a stateful shell or batch script.
  (To run a full script, use the :command:`configure_file` command or the
  :command:`file(GENERATE)` command to create it, and then specify
  a ``COMMAND`` to launch it.)
  The optional ``ARGS`` argument is for backward compatibility and
  will be ignored.

  If ``COMMAND`` specifies an executable target (created by the
  :command:`add_executable` command) it will automatically be replaced
  by the location of the executable created at build time.
  (Use the ``TARGET_FILE``
  :manual:`generator expression <cmake-generator-expressions(7)>` to
  reference an executable later in the command line.)
  Additionally a target-level dependency will be added so that the
  executable target will be built before any target using this custom
  command.  However this does NOT add a file-level dependency that
  would cause the custom command to re-run whenever the executable is
  recompiled.

  Arguments to ``COMMAND`` may use
  :manual:`generator expressions <cmake-generator-expressions(7)>`.
  References to target names in generator expressions imply target-level
  dependencies, but NOT file-level dependencies.  List target names with
  the ``DEPENDS`` option to add file-level dependencies.

``COMMENT``
  Display the given message before the commands are executed at
  build time.

``DEPENDS``
  Specify files on which the command depends.  If any dependency is
  an ``OUTPUT`` of another custom command in the same directory
  (``CMakeLists.txt`` file) CMake automatically brings the other
  custom command into the target in which this command is built.
  If ``DEPENDS`` is not specified the command will run whenever
  the ``OUTPUT`` is missing; if the command does not actually
  create the ``OUTPUT`` then the rule will always run.
  If ``DEPENDS`` specifies any target (created by the
  :command:`add_custom_target`, :command:`add_executable`, or
  :command:`add_library` command) a target-level dependency is
  created to make sure the target is built before any target
  using this custom command.  Additionally, if the target is an
  executable or library a file-level dependency is created to
  cause the custom command to re-run whenever the target is
  recompiled.

  Arguments to ``DEPENDS`` may use
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

``IMPLICIT_DEPENDS``
  Request scanning of implicit dependencies of an input file.
  The language given specifies the programming language whose
  corresponding dependency scanner should be used.
  Currently only ``C`` and ``CXX`` language scanners are supported.
  The language has to be specified for every file in the
  ``IMPLICIT_DEPENDS`` list.  Dependencies discovered from the
  scanning are added to those of the custom command at build time.
  Note that the ``IMPLICIT_DEPENDS`` option is currently supported
  only for Makefile generators and will be ignored by other generators.

``MAIN_DEPENDENCY``
  Specify the primary input source file to the command.  This is
  treated just like any value given to the ``DEPENDS`` option
  but also suggests to Visual Studio generators where to hang
  the custom command.  At most one custom command may specify a
  given source file as its main dependency.

``OUTPUT``
  Specify the output files the command is expected to produce.
  If an output name is a relative path it will be interpreted
  relative to the build tree directory corresponding to the
  current source directory.
  Each output file will be marked with the :prop_sf:`GENERATED`
  source file property automatically.
  If the output of the custom command is not actually created
  as a file on disk it should be marked with the :prop_sf:`SYMBOLIC`
  source file property.

``USES_TERMINAL``
  The command will be given direct access to the terminal if possible.
  With the :generator:`Ninja` generator, this places the command in
  the ``console`` :prop_gbl:`pool <JOB_POOLS>`.

``VERBATIM``
  All arguments to the commands will be escaped properly for the
  build tool so that the invoked command receives each argument
  unchanged.  Note that one level of escapes is still used by the
  CMake language processor before add_custom_command even sees the
  arguments.  Use of ``VERBATIM`` is recommended as it enables
  correct behavior.  When ``VERBATIM`` is not given the behavior
  is platform specific because there is no protection of
  tool-specific special characters.

``WORKING_DIRECTORY``
  Execute the command with the given current working directory.
  If it is a relative path it will be interpreted relative to the
  build tree directory corresponding to the current source directory.

Build Events
^^^^^^^^^^^^

The second signature adds a custom command to a target such as a
library or executable.  This is useful for performing an operation
before or after building the target.  The command becomes part of the
target and will only execute when the target itself is built.  If the
target is already built, the command will not execute.

::

  add_custom_command(TARGET target
                     PRE_BUILD | PRE_LINK | POST_BUILD
                     COMMAND command1 [ARGS] [args1...]
                     [COMMAND command2 [ARGS] [args2...] ...]
                     [BYPRODUCTS [files...]]
                     [WORKING_DIRECTORY dir]
                     [COMMENT comment]
                     [VERBATIM] [USES_TERMINAL])

This defines a new command that will be associated with building the
specified target.  When the command will happen is determined by which
of the following is specified:

``PRE_BUILD``
  Run before any other rules are executed within the target.
  This is supported only on Visual Studio 7 or later.
  For all other generators ``PRE_BUILD`` will be treated as
  ``PRE_LINK``.
``PRE_LINK``
  Run after sources have been compiled but before linking the binary
  or running the librarian or archiver tool of a static library.
  This is not defined for targets created by the
  :command:`add_custom_target` command.
``POST_BUILD``
  Run after all other rules within the target have been executed.
