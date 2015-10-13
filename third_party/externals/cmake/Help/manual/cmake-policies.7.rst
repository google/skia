.. cmake-manual-description: CMake Policies Reference

cmake-policies(7)
*****************

.. only:: html

   .. contents::

Introduction
============

Policies in CMake are used to preserve backward compatible behavior
across multiple releases.  When a new policy is introduced, newer CMake
versions will begin to warn about the backward compatible behavior.  It
is possible to disable the warning by explicitly requesting the OLD, or
backward compatible behavior using the :command:`cmake_policy` command.
It is also possible to request ``NEW``, or non-backward compatible behavior
for a policy, also avoiding the warning.  Each policy can also be set to
either ``NEW`` or ``OLD`` behavior explicitly on the command line with the
:variable:`CMAKE_POLICY_DEFAULT_CMP<NNNN>` variable.

A policy is a deprecation mechanism and not a reliable feature toggle.
A policy should almost never be set to ``OLD``, except to silence warnings
in an otherwise frozen or stable codebase, or temporarily as part of a
larger migration path. The ``OLD`` behavior of each policy is undesirable
and will be replaced with an error condition in a future release.

The :command:`cmake_minimum_required` command does more than report an
error if a too-old version of CMake is used to build a project.  It
also sets all policies introduced in that CMake version or earlier to
``NEW`` behavior.  To manage policies without increasing the minimum required
CMake version, the :command:`if(POLICY)` command may be used:

.. code-block:: cmake

  if(POLICY CMP0990)
    cmake_policy(SET CMP0990 NEW)
  endif()

This has the effect of using the ``NEW`` behavior with newer CMake releases which
users may be using and not issuing a compatibility warning.

The setting of a policy is confined in some cases to not propagate to the
parent scope.  For example, if the files read by the :command:`include` command
or the :command:`find_package` command contain a use of :command:`cmake_policy`,
that policy setting will not affect the caller by default.  Both commands accept
an optional ``NO_POLICY_SCOPE`` keyword to control this behavior.

The :variable:`CMAKE_MINIMUM_REQUIRED_VERSION` variable may also be used
to determine whether to report an error on use of deprecated macros or
functions.

All Policies
============

.. toctree::
   :maxdepth: 1

   /policy/CMP0000
   /policy/CMP0001
   /policy/CMP0002
   /policy/CMP0003
   /policy/CMP0004
   /policy/CMP0005
   /policy/CMP0006
   /policy/CMP0007
   /policy/CMP0008
   /policy/CMP0009
   /policy/CMP0010
   /policy/CMP0011
   /policy/CMP0012
   /policy/CMP0013
   /policy/CMP0014
   /policy/CMP0015
   /policy/CMP0016
   /policy/CMP0017
   /policy/CMP0018
   /policy/CMP0019
   /policy/CMP0020
   /policy/CMP0021
   /policy/CMP0022
   /policy/CMP0023
   /policy/CMP0024
   /policy/CMP0025
   /policy/CMP0026
   /policy/CMP0027
   /policy/CMP0028
   /policy/CMP0029
   /policy/CMP0030
   /policy/CMP0031
   /policy/CMP0032
   /policy/CMP0033
   /policy/CMP0034
   /policy/CMP0035
   /policy/CMP0036
   /policy/CMP0037
   /policy/CMP0038
   /policy/CMP0039
   /policy/CMP0040
   /policy/CMP0041
   /policy/CMP0042
   /policy/CMP0043
   /policy/CMP0044
   /policy/CMP0045
   /policy/CMP0046
   /policy/CMP0047
   /policy/CMP0048
   /policy/CMP0049
   /policy/CMP0050
   /policy/CMP0051
   /policy/CMP0052
   /policy/CMP0053
   /policy/CMP0054
   /policy/CMP0055
   /policy/CMP0056
   /policy/CMP0057
   /policy/CMP0058
   /policy/CMP0059
   /policy/CMP0060
   /policy/CMP0061
   /policy/CMP0062
   /policy/CMP0063
