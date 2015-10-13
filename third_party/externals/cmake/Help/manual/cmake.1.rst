.. cmake-manual-description: CMake Command-Line Reference

cmake(1)
********

Synopsis
========

.. parsed-literal::

 cmake [<options>] (<path-to-source> | <path-to-existing-build>)
 cmake [(-D <var>=<value>)...] -P <cmake-script-file>
 cmake --build <dir> [<options>] [-- <build-tool-options>...]
 cmake -E <command> [<options>...]
 cmake --find-package <options>...

Description
===========

The "cmake" executable is the CMake command-line interface.  It may be
used to configure projects in scripts.  Project configuration settings
may be specified on the command line with the -D option.

CMake is a cross-platform build system generator.  Projects specify
their build process with platform-independent CMake listfiles included
in each directory of a source tree with the name CMakeLists.txt.
Users build a project by using CMake to generate a build system for a
native tool on their platform.

Options
=======

.. include:: OPTIONS_BUILD.txt

``-E <command> [<options>...]``
 See `Command-Line Tool Mode`_.

``-L[A][H]``
 List non-advanced cached variables.

 List cache variables will run CMake and list all the variables from
 the CMake cache that are not marked as INTERNAL or ADVANCED.  This
 will effectively display current CMake settings, which can then be
 changed with -D option.  Changing some of the variables may result
 in more variables being created.  If A is specified, then it will
 display also advanced variables.  If H is specified, it will also
 display help for each variable.

``--build <dir>``
 Build a CMake-generated project binary tree.

 This abstracts a native build tool's command-line interface with the
 following options:

 ::

   <dir>          = Project binary directory to be built.
   --target <tgt> = Build <tgt> instead of default targets.
   --config <cfg> = For multi-configuration tools, choose <cfg>.
   --clean-first  = Build target 'clean' first, then build.
                    (To clean only, use --target 'clean'.)
   --use-stderr   = Ignored.  Behavior is default in CMake >= 3.0.
   --             = Pass remaining options to the native tool.

 Run cmake --build with no options for quick help.

``-N``
 View mode only.

 Only load the cache.  Do not actually run configure and generate
 steps.

``-P <file>``
 Process script mode.

 Process the given cmake file as a script written in the CMake
 language.  No configure or generate step is performed and the cache
 is not modified.  If variables are defined using -D, this must be
 done before the -P argument.

``--find-package``
 Run in pkg-config like mode.

 Search a package using find_package() and print the resulting flags
 to stdout.  This can be used to use cmake instead of pkg-config to
 find installed libraries in plain Makefile-based projects or in
 autoconf-based projects (via share/aclocal/cmake.m4).

``--graphviz=[file]``
 Generate graphviz of dependencies, see CMakeGraphVizOptions.cmake for more.

 Generate a graphviz input file that will contain all the library and
 executable dependencies in the project.  See the documentation for
 CMakeGraphVizOptions.cmake for more details.

``--system-information [file]``
 Dump information about this system.

 Dump a wide range of information about the current system.  If run
 from the top of a binary tree for a CMake project it will dump
 additional information such as the cache, log files etc.

``--debug-trycompile``
 Do not delete the try_compile build tree. Only useful on one try_compile at a time.

 Do not delete the files and directories created for try_compile
 calls.  This is useful in debugging failed try_compiles.  It may
 however change the results of the try-compiles as old junk from a
 previous try-compile may cause a different test to either pass or
 fail incorrectly.  This option is best used for one try-compile at a
 time, and only when debugging.

``--debug-output``
 Put cmake in a debug mode.

 Print extra stuff during the cmake run like stack traces with
 message(send_error ) calls.

``--trace``
 Put cmake in trace mode.

 Print a trace of all calls made and from where with
 message(send_error ) calls.

``--warn-uninitialized``
 Warn about uninitialized values.

 Print a warning when an uninitialized variable is used.

``--warn-unused-vars``
 Warn about unused variables.

 Find variables that are declared or set, but not used.

``--no-warn-unused-cli``
 Don't warn about command line options.

 Don't find variables that are declared on the command line, but not
 used.

``--check-system-vars``
 Find problems with variable usage in system files.

 Normally, unused and uninitialized variables are searched for only
 in CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR.  This flag tells CMake to
 warn about other files as well.

.. include:: OPTIONS_HELP.txt

Command-Line Tool Mode
======================

CMake provides builtin command-line tools through the signature::

 cmake -E <command> [<options>...]

Run ``cmake -E`` or ``cmake -E help`` for a summary of commands.
Available commands are:

``chdir <dir> <cmd> [<arg>...]``
  Change the current working directory and run a command.

``compare_files <file1> <file2>``
  Check if file1 is same as file2.

``copy <file> <destination>``
  Copy file to destination (either file or directory).

``copy_directory <source> <destination>``
  Copy directory 'source' content to directory 'destination'.

``copy_if_different <in-file> <out-file>``
  Copy file if input has changed.

``echo [<string>...]``
  Displays arguments as text.

``echo_append [<string>...]``
  Displays arguments as text but no new line.

``env [--unset=NAME]... [NAME=VALUE]... COMMAND [ARG]...``
  Run command in a modified environment.

``environment``
  Display the current environment.

``make_directory <dir>``
  Create a directory.

``md5sum [<file>...]``
  Compute md5sum of files.

``remove [-f] [<file>...]``
  Remove the file(s), use ``-f`` to force it.

``remove_directory <dir>``
  Remove a directory and its contents.

``rename <oldname> <newname>``
  Rename a file or directory (on one volume).

``sleep <number>...``
  Sleep for given number of seconds.

``tar [cxt][vf][zjJ] file.tar [<options>...] [--] [<file>...]``
  Create or extract a tar or zip archive.  Options are:

  ``--``
    Stop interpreting options and treat all remaining arguments
    as file names even if they start in ``-``.
  ``--files-from=<file>``
    Read file names from the given file, one per line.
    Blank lines are ignored.  Lines may not start in ``-``
    except for ``--add-file=<name>`` to add files whose
    names start in ``-``.
  ``--mtime=<date>``
    Specify modification time recorded in tarball entries.
  ``--format=<format>``
    Specify the format of the archive to be created.
    Supported formats are: ``7zip``, ``gnutar``, ``pax``,
    ``paxr`` (restricted pax, default), and ``zip``.

``time <command> [<args>...]``
  Run command and return elapsed time.

``touch <file>``
  Touch a file.

``touch_nocreate <file>``
  Touch a file if it exists but do not create it.

UNIX-specific Command-Line Tools
--------------------------------

The following ``cmake -E`` commands are available only on UNIX:

``create_symlink <old> <new>``
  Create a symbolic link ``<new>`` naming ``<old>``.

Windows-specific Command-Line Tools
-----------------------------------

The following ``cmake -E`` commands are available only on Windows:

``delete_regv <key>``
  Delete Windows registry value.

``env_vs8_wince <sdkname>``
  Displays a batch file which sets the environment for the provided
  Windows CE SDK installed in VS2005.

``env_vs9_wince <sdkname>``
  Displays a batch file which sets the environment for the provided
  Windows CE SDK installed in VS2008.

``write_regv <key> <value>``
  Write Windows registry value.

See Also
========

.. include:: LINKS.txt
