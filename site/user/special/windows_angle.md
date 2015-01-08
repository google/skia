Using ANGLE on Windows
======================

Introduction
------------

ANGLE converts OpenGL ES 2.0 calls to DirectX 9 calls. These instructions
document how to use ANGLE instead of the native OpenGL backend on Windows.

Details
-------

Angle is now downloaded as a part of Skia according to the `DEPS` file.

Add `skia_angle=1` to your `GYP_DEFINES` environment variable.

Run:

    python gyp_skia

Remember
--------

In SampleApp you will need to use the 'D' key to get to the ANGLE backend unless you enable the `DEFAULT_TO_ANGLE` #define in `SampleApp.cpp`.

  * Use “--angle” to enable ANGLE in gm.

  * Use “--config ANGLE” to use ANGLE in bench.

  * Use "--config angle" to use ANGLE in bench_pictures and render_pictures.

ANGLE will automatically be compiled into the GLInterfaceValidation test.
