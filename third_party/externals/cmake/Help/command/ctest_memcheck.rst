ctest_memcheck
--------------

Perform the :ref:`CTest MemCheck Step` as a :ref:`Dashboard Client`.

::

  ctest_memcheck([BUILD <build-dir>] [APPEND]
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


Run tests with a dynamic analysis tool and store results in
``MemCheck.xml`` for submission with the :command:`ctest_submit`
command.

The options are the same as those for the :command:`ctest_test` command.
