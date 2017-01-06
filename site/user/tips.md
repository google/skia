Tips & FAQ
==========

+   [Bitmap Subsetting](#bitmap-subsetting)
+   [Capture a `.skp` file on a web page in Chromium](#skp-capture)
+   [Capture a `.mskp` file on a web page in Chromium](#mskp-capture)
+   [How to add hardware acceleration in Skia](#hw-acceleration)
+   [Does Skia support Font hinting?](#font-hinting)
+   [Does Skia shape text (kerning)?](#kerning)
+   [How do I add drop shadow on text?](#text-shadow)

* * *

<span id="bitmap-subsetting">Bitmap Subsetting</span>
-----------------------------------------------------

Taking a subset of a bitmap is effectively free - no pixels are copied or
memory is allocated. This allows Skia to offer an API that typically operates
on entire bitmaps; clients who want to operate on a subset of a bitmap can use
the following pattern, here being used to magnify a portion of an image with
drawBitmapNine():

    SkBitmap subset;
    bitmap.extractSubset(&subset, rect);
    canvas->drawBitmapNine(subset, ...);

[An example](https://fiddle.skia.org/c/@subset_example)


* * *

<span id="skp-capture">Capture a `.skp` file on a web page in Chromium</span>
-----------------------------------------------------------------------------

1.  Launch Chrome or Chromium with `--no-sandbox --enable-gpu-benchmarking`
2.  Open the JS console (ctrl-shift-J)
3.  Execute: `chrome.gpuBenchmarking.printToSkPicture('/tmp')`
    This returns "undefined" on success.

Open the resulting file in the [Skia Debugger](/dev/tools/debugger), rasterize it with `dm`,
or use Skia's `SampleApp` to view it:

<!--?prettify lang=sh?-->

    out/Release/dm --src skp --skps /tmp/layer_0.skp -w /tmp \
        --config 8888 gpu pdf --verbose
    ls -l /tmp/*/skp/layer_0.skp.*

    out/Release/SampleApp --picture /tmp/layer_0.skp

* * *

<span id="mskp-capture">Capture a `.mskp` file on a web page in Chromium</span>
-------------------------------------------------------------------------------

Multipage Skia Picture files capture the commands sent to produce PDFs
and printed documents.

1.  Launch Chrome or Chromium with `--no-sandbox --enable-gpu-benchmarking`
2.  Open the JS console (ctrl-shift-J)
3.  Execute: `chrome.gpuBenchmarking.printPagesToSkPictures('/tmp/filename.mskp')`
    This returns "undefined" on success.

Open the resulting file in the [Skia Debugger](/dev/tools/debugger) or
process it with `dm`.

<!--?prettify lang=sh?-->

    experimental/tools/mskp_parser.py /tmp/filename.mskp /tmp/filename.mskp.skp
    ls -l /tmp/filename.mskp.skp
    # open filename.mskp.skp in the debugger.

    out/Release/dm --src mskp --mskps /tmp/filename.mskp -w /tmp \
        --config pdf --verbose
    ls -l /tmp/pdf/mskp/filename.mskp.pdf

* * *

<span id="hw-acceleration">How to add hardware acceleration in Skia</span>
--------------------------------------------------------------------------

There are two ways Skia takes advantage of specific hardware.

1.  Subclass SkCanvas

    Since all drawing calls go through SkCanvas, those calls can be
    redirected to a different graphics API. SkGLCanvas has been
    written to direct its drawing calls to OpenGL. See src/gl/

2.  Custom bottleneck routines

    There are sets of bottleneck routines inside the blits of Skia
    that can be replace on a platform in order to take advantage of
    specific CPU features. One such example is the NEON SIMD
    instructions on ARM v7 devices. See src/opts/

* * *

<span id="font-hinting">Does Skia support Font hinting?</span>
--------------------------------------------------------------

Skia has a built-in font cache, but it does not know how to actual render font
files like TrueType into its cache. For that it relies on the platform to
supply an instance of SkScalerContext. This is Skia's abstract interface for
communicating with a font scaler engine. In src/ports you can see support
files for FreeType, Mac OS X, and Windows GDI font engines. Other font
engines can easily be supported in a like manner.


* * *

<span id="kerning">Does Skia shape text (kerning)?</span>
---------------------------------------------------------

No.  Skia provides interfaces to draw glyphs, but does not implement a
text shaper. Skia's client's often use
[HarfBuzz](http://www.freedesktop.org/wiki/Software/HarfBuzz/) to
generate the glyphs and their positions, including kerning.

[Here is an example of how to use Skia and HarfBuzz
together](https://github.com/aam/skiaex).  In the example, a
`SkTypeface` and a `hb_face_t` are created using the same `mmap()`ed
`.ttf` font file. The HarfBuzz face is used to shape unicode text into
a sequence of glyphs and positions, and the SkTypeface can then be
used to draw those glyphs.

* * *

<span id="text-shadow">How do I add drop shadow on text?</span>
---------------------------------------------------------------

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        const char text[] = "Skia";
        const SkScalar radius = 2.0f;
        const SkScalar xDrop = 2.0f;
        const SkScalar yDrop = 2.0f;
        const SkScalar x = 8.0f;
        const SkScalar y = 52.0f;
        const SkScalar textSize = 48.0f;
        const uint8_t blurAlpha = 127;
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(textSize);
        SkPaint blur(paint);
        blur.setAlpha(blurAlpha);
        blur.setMaskFilter(SkBlurMaskFilter::Make(
            kNormal_SkBlurStyle,
            SkBlurMaskFilter::ConvertRadiusToSigma(radius), 0));
        canvas->drawText(text, strlen(text), x + xDrop, y + yDrop, blur);
        canvas->drawText(text, strlen(text), x, y, paint);
    }

<a href='https://fiddle.skia.org/c/@text_shadow'><img src='https://fiddle.skia.org/i/@text_shadow_raster.png'></a>

* * *

<div style="margin-bottom:99%"></div>
