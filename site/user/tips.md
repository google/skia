Tips & FAQ
==========

+   [Gyp Options](#gypdefines)
+   [Bitmap Subsetting](#bitmap-subsetting)
+   [Capture a `.skp` file on a web page in Chromium](#skp-capture)
+   [How to add hardware acceleration in Skia](#hw-acceleration)
+   [Does Skia support Font hinting?](#font-hinting)
+   [Does Skia shape text (kerning)?](#kerning)

* * *

<span id="gypdefines"></span>

Gyp Options
-----------

When running `sync-and-gyp`, the `GYP_DEFINES` environment variable can
be used to change Skia’s compile-time settings, using a
space-separated list of key=value pairs. For example, to disable both
the Skia GPU backend and PDF backends, run it as follows:

<!--?prettify lang=sh?-->

    GYP_DEFINES='skia_gpu=0 skia_pdf=0' python bin/sync-and-gyp
    ninja -C out/Debug

Note: Setting enviroment variables in the Windows CMD.EXE shell [uses a
different syntax](/user/quick/windows#env).

You can also set environment variables such as `CC`, `CXX`,
`CFLAGS`, `CXXFLAGS`, or `CPPFLAGS` to control how Skia is compiled.
To build with clang, for example:

<!--?prettify lang=sh?-->

    CC='clang' CXX='clang++' python bin/sync-and-gyp
    ninja -C out/Debug

To build with clang and enable a compiler warning for unused parameters in C++
(but not C or assembly) code:

<!--?prettify lang=sh?-->

    CXXFLAGS='-Wunused-parameter' \
        CC='clang' CXX='clang++' python bin/sync-and-gyp
    ninja -C out/Debug


The `GYP_GENERATORS` environment variable can be used to set the
build systems that you want to use (as a comma-separated list).
The default is `'ninja,msvs-ninja'` on Windows, `'ninja,xcode'` on
Mac OS X, and just `'ninja'` on Linux.  For example, to generate
only Ninja files on Mac:

<!--?prettify lang=sh?-->

    GYP_GENERATORS='ninja' python bin/sync-and-gyp
    ninja -C out/Debug

Finally, the `SKIA_OUT` environment variable can be used to set
the path for the build directory.  The default is `out` inside the
top-level Skia source directory.  For example to test Skia with
two different compilers:

<!--?prettify lang=sh?-->

    CC='clang' CXX='clang++' SKIA_OUT=~/build/skia_clang python bin/sync-and-gyp
    CC='gcc'   CXX='g++'     SKIA_OUT=~/build/skia_gcc   python bin/sync-and-gyp
    ninja -C ~/build/skia_clang/Debug
    ninja -C ~/build/skia_gcc/Debug

* * *

<span id="bitmap-subsetting"></span>

Bitmap Subsetting
-----------------

Taking a subset of a bitmap is effectively free - no pixels are copied or
memory is allocated. This allows Skia to offer an API that typically operates
on entire bitmaps; clients who want to operate on a subset of a bitmap can use
the following pattern, here being used to magnify a portion of an image with
drawBitmapNine():

    SkBitmap subset;
    bitmap.extractSubset(&subset, rect);
    canvas->drawBitmapNine(subset, ...);

[An example](https://fiddle.skia.org/c/c91694020f0810994917b56c323e4559)

* * *

<span id="skp-capture"></span>

Capture a `.skp` file on a web page in Chromium
-----------------------------------------------

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

* * *

<span id="hw-acceleration"></span>

How to add hardware acceleration in Skia
----------------------------------------

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

<span id="font-hinting"></span>

Does Skia support Font hinting?
-------------------------------

Skia has a built-in font cache, but it does not know how to actual render font
files like TrueType into its cache. For that it relies on the platform to
supply an instance of SkScalerContext. This is Skia's abstract interface for
communicating with a font scaler engine. In src/ports you can see support
files for FreeType, Mac OS X, and Windows GDI font engines. Other font
engines can easily be supported in a like manner.


* * *

<span id="kerning"></span>

Does Skia shape text (kerning)?
-------------------------------

No.  Skia provides interfaces to draw glyphs, but does not implement a
text shaper. Skia's client's often use
[HarfBuzz](http://www.freedesktop.org/wiki/Software/HarfBuzz/) to
generate the glyphs and their positions, including kerning.

<div style="margin-bottom:99%"></div>
