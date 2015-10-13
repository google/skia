ctest_coverage
--------------

Perform the :ref:`CTest Coverage Step` as a :ref:`Dashboard Client`.

::

  ctest_coverage([BUILD <build-dir>] [APPEND]
                 [LABELS <label>...]
                 [RETURN_VALUE <result-var>]
                 [QUIET]
                 )

Collect coverage tool results and stores them in ``Coverage.xml``
for submission with the :command:`ctest_submit` command.

The options are:

``BUILD <build-dir>``
  Specify the top-level build directory.  If not given, the
  :variable:`CTEST_BINARY_DIRECTORY` variable is used.

``APPEND``
  Mark results for append to those previously submitted to a
  dashboard server since the last :command:`ctest_start` call.
  Append semantics are defined by the dashboard server in use.

``LABELS``
  Filter the coverage report to include only source files labeled
  with at least one of the labels specified.

``RETURN_VALUE <result-var>``
  Store in the ``<result-var>`` variable ``0`` if coverage tools
  ran without error and non-zero otherwise.

``QUIET``
  Suppress any CTest-specific non-error output that would have been
  printed to the console otherwise.  The summary indicating how many
  lines of code were covered is unaffected by this option.
