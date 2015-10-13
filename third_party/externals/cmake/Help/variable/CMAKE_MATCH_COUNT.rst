CMAKE_MATCH_COUNT
-----------------

The number of matches with the last regular expression.

When a regular expression match is used, CMake fills in ``CMAKE_MATCH_<n>``
variables with the match contents. The ``CMAKE_MATCH_COUNT`` variable holds
the number of match expressions when these are filled.
