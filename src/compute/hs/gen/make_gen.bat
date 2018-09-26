@echo off

::
::
::

set GETOPT=../../../spinel/ext/getopt

::
::
::

set SRC_C=^
main.c ^
networks_merging.c ^
networks_sorting.c ^
target_cuda.c ^
target_debug.c ^
target_glsl.c ^
target_opencl.c ^
transpose.c ^
../../common/util.c ^
%GETOPT%/getopt.c

::
::
::

cl ^
/Fe:hs_gen ^
/O2 ^
/DNDEBUG ^
/I../.. ^
/I%GETOPT% ^
%SRC_C%

::
::
::

del *.obj

::
::
::
