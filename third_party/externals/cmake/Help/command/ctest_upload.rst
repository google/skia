ctest_upload
------------

Upload files to a dashboard server as a :ref:`Dashboard Client`.

::

  ctest_upload(FILES <file>... [QUIET])

The options are:

``FILES <file>...``
  Specify a list of files to be sent along with the build results to the
  dashboard server.

``QUIET``
  Suppress any CTest-specific non-error output that would have been
  printed to the console otherwise.
