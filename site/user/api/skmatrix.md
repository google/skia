SkMatrix
========

*3x3 transforms*

<!-- Updated Mar 4, 2011 -->

Skia is a 2D graphics engine, but it supports a full 3x3
transformation matrix. This allow it to draw anything (bitmaps, text,
rectangles, paths) in perspective. SkCamera is a helper class that
models a camera in 3D, and can be used to generate the proper matrix
for a given 3D view of the plane.
