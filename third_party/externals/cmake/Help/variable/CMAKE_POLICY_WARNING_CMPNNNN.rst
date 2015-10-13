CMAKE_POLICY_WARNING_CMP<NNNN>
------------------------------

Explicitly enable or disable the warning when CMake Policy ``CMP<NNNN>``
is not set.  This is meaningful only for the few policies that do not
warn by default:

* ``CMAKE_POLICY_WARNING_CMP0025`` controls the warning for
  policy :policy:`CMP0025`.
* ``CMAKE_POLICY_WARNING_CMP0047`` controls the warning for
  policy :policy:`CMP0047`.
* ``CMAKE_POLICY_WARNING_CMP0056`` controls the warning for
  policy :policy:`CMP0056`.
* ``CMAKE_POLICY_WARNING_CMP0060`` controls the warning for
  policy :policy:`CMP0060`.

This variable should not be set by a project in CMake code.  Project
developers running CMake may set this variable in their cache to
enable the warning (e.g. ``-DCMAKE_POLICY_WARNING_CMP<NNNN>=ON``).
Alternatively, running :manual:`cmake(1)` with the ``--debug-output``
or ``--trace`` option will also enable the warning.
