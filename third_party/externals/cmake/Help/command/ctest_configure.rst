ctest_configure
---------------

Perform the :ref:`CTest Configure Step` as a :ref:`Dashboard Client`.

::

  ctest_configure([BUILD <build-dir>] [SOURCE <source-dir>] [APPEND]
                  [OPTIONS <options>] [RETURN_VALUE <result-var>] [QUIET])

Configure the project build tree and record results in ``Configure.xml``
for submission with the :command:`ctest_submit` command.

The options are:

``BUILD <build-dir>``
  Specify the top-level build directory.  If not given, the
  :variable:`CTEST_BINARY_DIRECTORY` variable is used.

``SOURCE <source-dir>``
  Specify the source directory.  If not given, the
  :variable:`CTEST_SOURCE_DIRECTORY` variable is used.

``APPEND``
  Mark results for append to those previously submitted to a
  dashboard server since the last :command:`ctest_start` call.
  Append semantics are defined by the dashboard server in use.

``OPTIONS <options>``
  Specify command-line arguments to pass to the configuration tool.

``RETURN_VALUE <result-var>``
  Store in the ``<result-var>`` variable the return value of the native
  configuration tool.

``QUIET``
  Suppress any CTest-specific non-error messages that would have
  otherwise been printed to the console.  Output from the underlying
  configure command is not affected.
