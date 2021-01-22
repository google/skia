API Reference and Overview
==========================

Skia documentation is actively under development.

Some key classes are:

*   [SkAutoCanvasRestore](https://api.skia.org/classSkAutoCanvasRestore.html#details) - Canvas save stack manager
*   [SkBitmap](https://api.skia.org/classSkBitmap.html#details) - two-dimensional raster pixel array
*   [SkBlendMode](https://api.skia.org/SkBlendMode_8h.html) - pixel color arithmetic
*   [SkCanvas](https://api.skia.org/classSkCanvas.html#details) - drawing context
*   [SkColor](https://api.skia.org/SkColor_8h.html) - color encoding using integer numbers
*   [SkFont](https://api.skia.org/classSkFont.html#details) - text style and typeface
*   [SkImage](https://api.skia.org/classSkImage.html#details) - two dimensional array of pixels to draw
*   [SkImageInfo](https://api.skia.org/structSkImageInfo.html#details) - pixel dimensions and characteristics
*   [SkIPoint](https://api.skia.org/structSkIPoint.html#details) - two integer coordinates
*   [SkIRect](https://api.skia.org/structSkIRect.html#details) - integer rectangle
*   [SkMatrix](https://api.skia.org/classSkMatrix.html#details) - 3x3 transformation matrix
*   [SkPaint](https://api.skia.org/classSkPaint.html#details) - color, stroke, font, effects
*   [SkPath](https://api.skia.org/classSkPath.html#details) - sequence of connected lines and curves
*   [SkPicture](https://api.skia.org/classSkPicture.html#details) - sequence of drawing commands
*   [SkPixmap](https://api.skia.org/classSkPixmap.html#details) - pixel map: image info and pixel address
*   [SkPoint](https://api.skia.org/structSkPoint.html#details) - two floating point coordinates
*   [SkRRect](https://api.skia.org/classSkRRect.html#details) - floating point rounded rectangle
*   [SkRect](https://api.skia.org/structSkRect.html#details) - floating point rectangle
*   [SkRegion](https://api.skia.org/classSkRegion.html#details) - compressed clipping mask
*   [SkSurface](https://api.skia.org/classSkSurface.html#details) - drawing destination
*   [SkTextBlob](https://api.skia.org/classSkTextBlob.html#details) - runs of glyphs
*   [SkTextBlobBuilder](https://api.skia.org/classSkTextBlobBuilder.html#details) - constructor for runs of glyphs

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

