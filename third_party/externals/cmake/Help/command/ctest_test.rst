ctest_test
----------

Perform the :ref:`CTest Test Step` as a :ref:`Dashboard Client`.

::

  ctest_test([BUILD <build-dir>] [APPEND]
             [START <start-number>]
             [END <end-number>]
             [STRIDE <stride-number>]
             [EXCLUDE <exclude-regex>]
             [INCLUDE <include-regex>]
             [EXCLUDE_LABEL <label-exclude-regex>]
             [INCLUDE_LABEL <label-include-regex>]
             [PARALLEL_LEVEL <level>]
             [SCHEDULE_RANDOM <ON|OFF>]
             [STOP_TIME <time-of-day>]
             [RETURN_VALUE <result-var>]
             [QUIET]
             )

Run tests in the project build tree and store results in
``Test.xml`` for submission with the :command:`ctest_submit` command.

The options are:

``BUILD <build-dir>``
  Specify the top-level build directory.  If not given, the
  :variable:`CTEST_BINARY_DIRECTORY` variable is used.

``APPEND``
  Mark results for append to those previously submitted to a
  dashboard server since the last :command:`ctest_start` call.
  Append semantics are defined by the dashboard server in use.

``START <start-number>``
  Specify the beginning of a range of test numbers.

``END <end-number>``
  Specify the end of a range of test numbers.

``STRIDE <stride-number>``
  Specify the stride by which to step acorss a range of test numbers.

``EXCLUDE <exclude-regex>``
  Specify a regular expression matching test names to exclude.

``INCLUDE <include-regex>``
  Specify a regular expression matching test names to include.
  Tests not matching this expression are excluded.

``EXCLUDE_LABEL <label-exclude-regex>``
  Specify a regular expression matching test labels to exclude.

``INCLUDE_LABEL <label-include-regex>``
  Specify a regular expression matching test labels to include.
  Tests not matching this expression are excluded.

``PARALLEL_LEVEL <level>``
  Specify a positive number representing the number of tests to
  be run in parallel.

``SCHEDULE_RANDOM <ON|OFF>``
  Launch tests in a random order.  This may be useful for detecting
  implicit test dependencies.

``STOP_TIME <time-of-day>``
  Specify a time of day at which the tests should all stop running.

``RETURN_VALUE <result-var>``
  Store in the ``<result-var>`` variable ``0`` if all tests passed.
  Store non-zero if anything went wrong.

``QUIET``
  Suppress any CTest-specific non-error messages that would have otherwise
  been printed to the console.  Output from the underlying test command is not
  affected.  Summary info detailing the percentage of passing tests is also
  unaffected by the ``QUIET`` option.
