file
----

File manipulation command.

------------------------------------------------------------------------------

::

  file(WRITE <filename> <content>...)
  file(APPEND <filename> <content>...)

Write ``<content>`` into a file called ``<filename>``.  If the file does
not exist, it will be created.  If the file already exists, ``WRITE``
mode will overwrite it and ``APPEND`` mode will append to the end.
(If the file is a build input, use the :command:`configure_file` command
to update the file only when its content changes.)

------------------------------------------------------------------------------

::

  file(READ <filename> <variable>
       [OFFSET <offset>] [LIMIT <max-in>] [HEX])

Read content from a file called ``<filename>`` and store it in a
``<variable>``.  Optionally start from the given ``<offset>`` and
read at most ``<max-in>`` bytes.  The ``HEX`` option causes data to
be converted to a hexadecimal representation (useful for binary data).

------------------------------------------------------------------------------

::

  file(STRINGS <filename> <variable> [<options>...])

Parse a list of ASCII strings from ``<filename>`` and store it in
``<variable>``.  Binary data in the file are ignored.  Carriage return
(``\r``, CR) characters are ignored.  The options are:

``LENGTH_MAXIMUM <max-len>``
 Consider only strings of at most a given length.

``LENGTH_MINIMUM <min-len>``
 Consider only strings of at least a given length.

``LIMIT_COUNT <max-num>``
 Limit the number of distinct strings to be extracted.

``LIMIT_INPUT <max-in>``
 Limit the number of input bytes to read from the file.

``LIMIT_OUTPUT <max-out>``
 Limit the number of total bytes to store in the ``<variable>``.

``NEWLINE_CONSUME``
 Treat newline characters (``\n``, LF) as part of string content
 instead of terminating at them.

``NO_HEX_CONVERSION``
 Intel Hex and Motorola S-record files are automatically converted to
 binary while reading unless this option is given.

``REGEX <regex>``
 Consider only strings that match the given regular expression.

``ENCODING <encoding-type>``
 Consider strings of a given encoding.  Currently supported encodings are:
 UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE.  If the ENCODING option
 is not provided and the file has a Byte Order Mark, the ENCODING option
 will be defaulted to respect the Byte Order Mark.

For example, the code

.. code-block:: cmake

  file(STRINGS myfile.txt myfile)

stores a list in the variable ``myfile`` in which each item is a line
from the input file.

------------------------------------------------------------------------------

::

  file(<MD5|SHA1|SHA224|SHA256|SHA384|SHA512> <filename> <variable>)

Compute a cryptographic hash of the content of ``<filename>`` and
store it in a ``<variable>``.

------------------------------------------------------------------------------

::

  file(GLOB <variable>
       [LIST_DIRECTORIES true|false] [RELATIVE <path>]
       [<globbing-expressions>...])
  file(GLOB_RECURSE <variable> [FOLLOW_SYMLINKS]
       [LIST_DIRECTORIES true|false] [RELATIVE <path>]
       [<globbing-expressions>...])

Generate a list of files that match the ``<globbing-expressions>`` and
store it into the ``<variable>``.  Globbing expressions are similar to
regular expressions, but much simpler.  If ``RELATIVE`` flag is
specified, the results will be returned as relative paths to the given
path.

By default ``GLOB`` lists directories - directories are omited in result if
``LIST_DIRECTORIES`` is set to false.

.. note::
  We do not recommend using GLOB to collect a list of source files from
  your source tree.  If no CMakeLists.txt file changes when a source is
  added or removed then the generated build system cannot know when to
  ask CMake to regenerate.

Examples of globbing expressions include::

  *.cxx      - match all files with extension cxx
  *.vt?      - match all files with extension vta,...,vtz
  f[3-5].txt - match files f3.txt, f4.txt, f5.txt

The ``GLOB_RECURSE`` mode will traverse all the subdirectories of the
matched directory and match the files.  Subdirectories that are symlinks
are only traversed if ``FOLLOW_SYMLINKS`` is given or policy
:policy:`CMP0009` is not set to ``NEW``.

By default ``GLOB_RECURSE`` omits directories from result list - setting
``LIST_DIRECTORIES`` to true adds directories to result list.
If ``FOLLOW_SYMLINKS`` is given or policy :policy:`CMP0009` is not set to
``OLD`` then ``LIST_DIRECTORIES`` treats symlinks as directories.

Examples of recursive globbing include::

  /dir/*.py  - match all python files in /dir and subdirectories

------------------------------------------------------------------------------

::

  file(RENAME <oldname> <newname>)

Move a file or directory within a filesystem from ``<oldname>`` to
``<newname>``, replacing the destination atomically.

------------------------------------------------------------------------------

::

  file(REMOVE [<files>...])
  file(REMOVE_RECURSE [<files>...])

Remove the given files.  The ``REMOVE_RECURSE`` mode will remove the given
files and directories, also non-empty directories

------------------------------------------------------------------------------

::

  file(MAKE_DIRECTORY [<directories>...])

Create the given directories and their parents as needed.

------------------------------------------------------------------------------

::

  file(RELATIVE_PATH <variable> <directory> <file>)

Compute the relative path from a ``<directory>`` to a ``<file>`` and
store it in the ``<variable>``.

------------------------------------------------------------------------------

::

  file(TO_CMAKE_PATH "<path>" <variable>)
  file(TO_NATIVE_PATH "<path>" <variable>)

The ``TO_CMAKE_PATH`` mode converts a native ``<path>`` into a cmake-style
path with forward-slashes (``/``).  The input can be a single path or a
system search path like ``$ENV{PATH}``.  A search path will be converted
to a cmake-style list separated by ``;`` characters.

The ``TO_NATIVE_PATH`` mode converts a cmake-style ``<path>`` into a native
path with platform-specific slashes (``\`` on Windows and ``/`` elsewhere).

Always use double quotes around the ``<path>`` to be sure it is treated
as a single argument to this command.

------------------------------------------------------------------------------

::

  file(DOWNLOAD <url> <file> [<options>...])
  file(UPLOAD   <file> <url> [<options>...])

The ``DOWNLOAD`` mode downloads the given ``<url>`` to a local ``<file>``.
The ``UPLOAD`` mode uploads a local ``<file>`` to a given ``<url>``.

Options to both ``DOWNLOAD`` and ``UPLOAD`` are:

``INACTIVITY_TIMEOUT <seconds>``
  Terminate the operation after a period of inactivity.

``LOG <variable>``
  Store a human-readable log of the operation in a variable.

``SHOW_PROGRESS``
  Print progress information as status messages until the operation is
  complete.

``STATUS <variable>``
  Store the resulting status of the operation in a variable.
  The status is a ``;`` separated list of length 2.
  The first element is the numeric return value for the operation,
  and the second element is a string value for the error.
  A ``0`` numeric error means no error in the operation.

``TIMEOUT <seconds>``
  Terminate the operation after a given total time has elapsed.

Additional options to ``DOWNLOAD`` are:

``EXPECTED_HASH ALGO=<value>``

  Verify that the downloaded content hash matches the expected value, where
  ``ALGO`` is one of ``MD5``, ``SHA1``, ``SHA224``, ``SHA256``, ``SHA384``, or
  ``SHA512``.  If it does not match, the operation fails with an error.

``EXPECTED_MD5 <value>``
  Historical short-hand for ``EXPECTED_HASH MD5=<value>``.

``TLS_VERIFY <ON|OFF>``
  Specify whether to verify the server certificate for ``https://`` URLs.
  The default is to *not* verify.

``TLS_CAINFO <file>``
  Specify a custom Certificate Authority file for ``https://`` URLs.

For ``https://`` URLs CMake must be built with OpenSSL support.  ``TLS/SSL``
certificates are not checked by default.  Set ``TLS_VERIFY`` to ``ON`` to
check certificates and/or use ``EXPECTED_HASH`` to verify downloaded content.
If neither ``TLS`` option is given CMake will check variables
``CMAKE_TLS_VERIFY`` and ``CMAKE_TLS_CAINFO``, respectively.

------------------------------------------------------------------------------

::

  file(TIMESTAMP <filename> <variable> [<format>] [UTC])

Compute a string representation of the modification time of ``<filename>``
and store it in ``<variable>``.  Should the command be unable to obtain a
timestamp variable will be set to the empty string ("").

See the :command:`string(TIMESTAMP)` command for documentation of
the ``<format>`` and ``UTC`` options.

------------------------------------------------------------------------------

::

  file(GENERATE OUTPUT output-file
       <INPUT input-file|CONTENT content>
       [CONDITION expression])

Generate an output file for each build configuration supported by the current
:manual:`CMake Generator <cmake-generators(7)>`.  Evaluate
:manual:`generator expressions <cmake-generator-expressions(7)>`
from the input content to produce the output content.  The options are:

``CONDITION <condition>``
  Generate the output file for a particular configuration only if
  the condition is true.  The condition must be either ``0`` or ``1``
  after evaluating generator expressions.

``CONTENT <content>``
  Use the content given explicitly as input.

``INPUT <input-file>``
  Use the content from a given file as input.

``OUTPUT <output-file>``
  Specify the output file name to generate.  Use generator expressions
  such as ``$<CONFIG>`` to specify a configuration-specific output file
  name.  Multiple configurations may generate the same output file only
  if the generated content is identical.  Otherwise, the ``<output-file>``
  must evaluate to an unique name for each configuration.

Exactly one ``CONTENT`` or ``INPUT`` option must be given.  A specific
``OUTPUT`` file may be named by at most one invocation of ``file(GENERATE)``.
Generated files are modified on subsequent cmake runs only if their content
is changed.

------------------------------------------------------------------------------

::

  file(<COPY|INSTALL> <files>... DESTINATION <dir>
       [FILE_PERMISSIONS <permissions>...]
       [DIRECTORY_PERMISSIONS <permissions>...]
       [NO_SOURCE_PERMISSIONS] [USE_SOURCE_PERMISSIONS]
       [FILES_MATCHING]
       [[PATTERN <pattern> | REGEX <regex>]
        [EXCLUDE] [PERMISSIONS <permissions>...]] [...])

The ``COPY`` signature copies files, directories, and symlinks to a
destination folder.  Relative input paths are evaluated with respect
to the current source directory, and a relative destination is
evaluated with respect to the current build directory.  Copying
preserves input file timestamps, and optimizes out a file if it exists
at the destination with the same timestamp.  Copying preserves input
permissions unless explicit permissions or ``NO_SOURCE_PERMISSIONS``
are given (default is ``USE_SOURCE_PERMISSIONS``).

See the :command:`install(DIRECTORY)` command for documentation of
permissions, ``FILES_MATCHING``, ``PATTERN``, ``REGEX``, and
``EXCLUDE`` options.  Copying directories preserves the structure
of their content even if options are used to select a subset of
files.

The ``INSTALL`` signature differs slightly from ``COPY``: it prints
status messages (subject to the :variable:`CMAKE_INSTALL_MESSAGE` variable),
and ``NO_SOURCE_PERMISSIONS`` is default.
Installation scripts generated by the :command:`install` command
use this signature (with some undocumented options for internal use).

------------------------------------------------------------------------------

::

  file(LOCK <path> [DIRECTORY] [RELEASE]
       [GUARD <FUNCTION|FILE|PROCESS>]
       [RESULT_VARIABLE <variable>]
       [TIMEOUT <seconds>])

Lock a file specified by ``<path>`` if no ``DIRECTORY`` option present and file
``<path>/cmake.lock`` otherwise. File will be locked for scope defined by
``GUARD`` option (default value is ``PROCESS``). ``RELEASE`` option can be used
to unlock file explicitly. If option ``TIMEOUT`` is not specified CMake will
wait until lock succeed or until fatal error occurs. If ``TIMEOUT`` is set to
``0`` lock will be tried once and result will be reported immediately. If
``TIMEOUT`` is not ``0`` CMake will try to lock file for the period specified
by ``<seconds>`` value. Any errors will be interpreted as fatal if there is no
``RESULT_VARIABLE`` option. Otherwise result will be stored in ``<variable>``
and will be ``0`` on success or error message on failure.

Note that lock is advisory - there is no guarantee that other processes will
respect this lock, i.e. lock synchronize two or more CMake instances sharing
some modifiable resources. Similar logic applied to ``DIRECTORY`` option -
locking parent directory doesn't prevent other ``LOCK`` commands to lock any
child directory or file.

Trying to lock file twice is not allowed.  Any intermediate directories and
file itself will be created if they not exist.  ``GUARD`` and ``TIMEOUT``
options ignored on ``RELEASE`` operation.
