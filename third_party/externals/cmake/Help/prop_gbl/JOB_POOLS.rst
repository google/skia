JOB_POOLS
---------

Ninja only: List of available pools.

A pool is a named integer property and defines the maximum number
of concurrent jobs which can be started by a rule assigned to the pool.
The :prop_gbl:`JOB_POOLS` property is a semicolon-separated list of
pairs using the syntax NAME=integer (without a space after the equality sign).

For instance:

.. code-block:: cmake

  set_property(GLOBAL PROPERTY JOB_POOLS two_jobs=2 ten_jobs=10)

Defined pools could be used globally by setting
:variable:`CMAKE_JOB_POOL_COMPILE` and :variable:`CMAKE_JOB_POOL_LINK`
or per target by setting the target properties
:prop_tgt:`JOB_POOL_COMPILE` and :prop_tgt:`JOB_POOL_LINK`.

Build targets provided by CMake that are meant for individual interactive
use, such as ``install``, are placed in the ``console`` pool automatically.
