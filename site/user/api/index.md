API Reference and Overview
==========================

Skia documentation is actively under development.

Full references with examples are available for:

*   [SkAutoCanvasRestore](/user/api/SkAutoCanvasRestore_Reference) - Canvas save stack manager
*   [SkBitmap](/user/api/SkBitmap_Reference) - two-dimensional raster pixel array
*   [SkBlendMode](/user/api/SkBlendMode_Reference) - pixel color arithmetic
*   [SkCanvas](/user/api/SkCanvas_Reference) - drawing context
*   [SkColor](/user/api/SkColor_Reference) - color encoding using integer numbers
*   [SkColor4f](/user/api/SkColor4f_Reference) - color encoding using floating point numbers
*   [SkFont](/user/api/SkFont_Reference) - text style and typeface
*   [SkImage](/user/api/SkImage_Reference) - two dimensional array of pixels to draw
*   [SkImageInfo](/user/api/SkImageInfo_Reference) - pixel dimensions and characteristics
*   [SkIPoint](/user/api/SkIPoint_Reference) - two integer coordinates
*   [SkIRect](/user/api/SkIRect_Reference) - integer rectangle
*   [SkMatrix](/user/api/SkMatrix_Reference) - 3x3 transformation matrix
*   [SkPaint](/user/api/SkPaint_Reference) - color, stroke, font, effects
*   [SkPath](/user/api/SkPath_Reference) - sequence of connected lines and curves
*   [SkPicture](/user/api/SkPicture_Reference) - sequence of drawing commands
*   [SkPixmap](/user/api/SkPixmap_Reference) - pixel map: image info and pixel address
*   [SkPoint](/user/api/SkPoint_Reference) - two floating point coordinates
*   [SkRRect](/user/api/SkRRect_Reference) - floating point rounded rectangle
*   [SkRect](/user/api/SkRect_Reference) - floating point rectangle
*   [SkRegion](/user/api/SkRegion_Reference) - compressed clipping mask
*   [SkSurface](/user/api/SkSurface_Reference) - drawing destination
*   [SkTextBlob](/user/api/SkTextBlob_Reference) - runs of glyphs
*   [SkTextBlobBuilder](/user/api/SkTextBlobBuilder_Reference) - constructor for runs of glyphs

Check out [a graphical overview of examples](api/catalog.htm)

All public APIs are indexed by Doxygen.

*   [Skia Doxygen](https://api.skia.org)

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

1.  [SkCanvas Overview](/user/api/skcanvas_overview) - the drawing context
2.  [SkPaint Overview](/user/api/skpaint_overview) - color, stroke, font, effects
3.  [SkCanvas Creation](/user/api/skcanvas_creation)

