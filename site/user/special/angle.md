ANGLE
=====

Introduction
------------

ANGLE converts OpenGL ES 2 or 3 calls to DirectX 9, 11, or OpenGL calls. These
instructions document how to use ANGLE instead of the native OpenGL backend on
Windows or Linux.

Details
-------

`gclient sync` downloads ANGLE's source alongside Skia's other test-only dependencies.

To build Skia testing tools against ANGLE, add `skia_use_angle = true` to your
`args.gn` file (or run `gn args` to edit it).

When running tools, use `--config angle_<backend>_<frontend>`, e.g.

    out/Debug/dm --src gm --config angle_d3d11_es2
    out/Release/nanobench --config angle_gl_es2
