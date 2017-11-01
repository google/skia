API Reference and Overview
==========================

Skia documentation is actively under development.

Full references with examples are available for:

*  [SkBitmap](/user/api/SkBitmap_Reference) - two-dimensional raster pixel array
*  [SkCanvas](/user/api/SkCanvas_Reference) - drawing context
*  [SkIRect](/user/api/SkIRect_Reference) - integer rectangle
*  [SkMatrix](/user/api/SkMatrix_Reference) - 3x3 transformation matrix
*  [SkPaint](/user/api/SkPaint_Reference) - color, stroke, font, effects
*  [SkPath](/user/api/SkPath_Reference) - series of connected lines and curves
*  [SkPixmap](/user/api/SkPixmap_Reference) - pixel map: image info and pixel address
*  [SkRect](/user/api/SkRect_Reference) - floating point rectangle

Check out [a graphical overview of examples](api/catalog.htm)

All public APIs are indexed by Doxygen. The Doxyen index is current, but the
content is dated and incomplete. Doxygen content will be superceded by 
full references with examples.

*   [Skia Doxygen](http://skia-doc.commondatastorage.googleapis.com/doxygen/doxygen/html/index.html)

## Overview

Skia is organized around the `SkCanvas` object. It is the host for the
"draw" calls: `drawRect`, `drawPath`, `drawText`, etc. Each of these
has two components: the primitive being drawn (`SkRect`, `SkPath`, etc.)
and color/style attributes (`SkPaint`).

<!--?prettify lang=cc?-->

    canvas->drawRect(rect, paint);

The paint holds much of the state describing how the rectangle (in
this case) is drawn: what color it is, if it is filled or stroked, how
it should blend with what was previously drawn.

The canvas holds relatively little state. It points to the actual
pixels being drawn, and it maintains a stack of matrices and
clips. Thus in the above call, the canvas' current matrix may
transform the coordinates of the rectangle (translation, rotation,
skewing, perspective), and the canvas' current clip may restrict where
on the canvas the rectangle will be drawn, but all other stylistic
attributes of the drawing are controlled by the paint.

Using the SkCanvas API:

*  [SkCanvas Overview](/user/api/skcanvas_overivew) - the drawing context
*  [Creating SkCanvas Objects](/user/api/creating_skcanvas)
*  [SkPaint Overview](/user/api/skpaint_overview) - color, stroke, font, effects
