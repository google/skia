Tips & FAQ
==========

Tips and Tricks
---------------

### Bitmap Subsetting

Taking a subset of a bitmap is effectively free - no pixels are copied or
memory is allocated. This allows Skia to offer an API that typically operates
on entire bitmaps; clients who want to operate on a subset of a bitmap can use
the following pattern, here being used to magnify a portion of an image with
drawBitmapNine():

    SkBitmap subset;
    bitmap.extractSubset(&subset, rect);
    canvas->drawBitmapNine(subset, ...);

FAQ
---

### Does Skia support HW acceleration?


There are two ways Skia can take advantage of HW.

1. Subclass SkCanvas

Since all drawing calls go through SkCanvas, those calls can be redirected to
a different graphics API. SkGLCanvas has been written to direct its drawing
calls to OpenGL. See src/gl/

2. Custom bottleneck routines

There are sets of bottleneck routines inside the blits of Skia that can be
replace on a platform in order to take advantage of specific CPU features. One
such example is the NEON SIMD instructions on ARM v7 devices. See src/opts/

### Does Skia support Font hinting?

Skia has a built-in font cache, but it does not know how to actual render font
files like TrueType? into its cache. For that it relies on the platform to
supply an instance of SkScalerContext?. This is Skia's abstract interface for
communicating with a font scaler engine. In src/ports you can see support
files for FreeType?, Mac OS X, and Windows GDI font engines. Other font
engines can easily be supported in a like manner.


