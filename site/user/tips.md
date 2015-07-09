Tips & FAQ
==========

Tips and Tricks
---------------

<span id="bitmap-subsetting"></span>

### Bitmap Subsetting

Taking a subset of a bitmap is effectively free - no pixels are copied or
memory is allocated. This allows Skia to offer an API that typically operates
on entire bitmaps; clients who want to operate on a subset of a bitmap can use
the following pattern, here being used to magnify a portion of an image with
drawBitmapNine():

    SkBitmap subset;
    bitmap.extractSubset(&subset, rect);
    canvas->drawBitmapNine(subset, ...);

* * *

<span id="skp-capture"></span>

### Capturing a `.skp` file on a web page in Chromium.


1.  Launch Chrome or Chromium with `--no-sandbox --enable-gpu-benchmarking`
2.  Open the JS console (ctrl-shift-J)
3.  Execute: `chrome.gpuBenchmarking.printToSkPicture('/tmp')`
    This returns "undefined" on success.

Open the resulting file in the Skia Debugger, rasterize it with `dm`,
or use Skia's `SampleApp` to view it:

<!--?prettify lang=sh?-->

    bin/sync-and-gyp
    ninja -C out/Release debugger dm SampleApp
    out/Release/debugger /tmp/layer_0.skp &

    out/Release/dm --src skp --skps /tmp/layer_0.skp -w /tmp \
        --config 8888 gpu pdf --verbose
    ls -l /tmp/*/skp/layer_0.skp.*

    out/Release/SampleApp --picture /tmp/layer_0.skp
    # On MacOS, SampleApp is a bundle:
    open out/Release/SampleApp.app --args --picture /tmp/layer_0.skp

* * *

FAQ
---

<span id="hw-acceleration"></span>

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

* * *

<span id="font-hinting"></span>

### Does Skia support Font hinting?

Skia has a built-in font cache, but it does not know how to actual render font
files like TrueType? into its cache. For that it relies on the platform to
supply an instance of SkScalerContext?. This is Skia's abstract interface for
communicating with a font scaler engine. In src/ports you can see support
files for FreeType?, Mac OS X, and Windows GDI font engines. Other font
engines can easily be supported in a like manner.


