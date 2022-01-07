---
title: 'Tips & FAQ'
linkTitle: 'Tips & FAQ'
---

## Capture a `.skp` file on a web page in Chromium

Use the script `experimental/tools/web_to_skp` , _or_ do the following:

1.  Launch Chrome or Chromium with `--no-sandbox --enable-gpu-benchmarking`
2.  Open the JS console (Ctrl+Shift+J (Windows / Linux) or Cmd+Opt+J (MacOS))
3.  Execute: `chrome.gpuBenchmarking.printToSkPicture('/tmp')` This returns
    "undefined" on success.

Open the resulting file in the [Skia Debugger](/docs/dev/tools/debugger),
rasterize it with `dm`, or use Skia's `viewer` to view it:

<!--?prettify lang=sh?-->

    out/Release/dm --src skp --skps /tmp/layer_0.skp -w /tmp \
        --config 8888 gpu pdf --verbose
    ls -l /tmp/*/skp/layer_0.skp.*

    out/Release/viewer --skps /tmp --slide layer_0.skp

---

## Capture a `.mskp` file on a web page in Chromium

Multipage Skia Picture files capture the commands sent to produce PDFs and
printed documents.

Use the script `experimental/tools/web_to_mskp` , _or_ do the following:

1.  Launch Chrome or Chromium with `--no-sandbox --enable-gpu-benchmarking`
2.  Open the JS console (Ctrl+Shift+J (Windows / Linux) or Cmd+Opt+J (MacOS))
3.  Execute:
    `chrome.gpuBenchmarking.printPagesToSkPictures('/tmp/filename.mskp')` This
    returns "undefined" on success.

Open the resulting file in the [Skia Debugger](/docs/dev/tools/debugger) or
process it with `dm`.

<!--?prettify lang=sh?-->

    experimental/tools/mskp_parser.py /tmp/filename.mskp /tmp/filename.mskp.skp
    ls -l /tmp/filename.mskp.skp
    # open filename.mskp.skp in the debugger.

    out/Release/dm --src mskp --mskps /tmp/filename.mskp -w /tmp \
        --config pdf --verbose
    ls -l /tmp/pdf/mskp/filename.mskp.pdf

---

## How to add hardware acceleration in Skia

There are two ways Skia takes advantage of specific hardware.

1.  Custom bottleneck routines

    There are sets of bottleneck routines inside the blits of Skia that can be
    replace on a platform in order to take advantage of specific CPU features.
    One such example is the NEON SIMD instructions on ARM v7 devices. See
    [src/opts/](https://skia.googlesource.com/skia/+/main/src/opts/)

---

## Does Skia support Font hinting?

Skia has a built-in font cache, but it does not know how to actually render font
files like TrueType into its cache. For that it relies on the platform to supply
an instance of `SkScalerContext`. This is Skia's abstract interface for
communicating with a font scaler engine. In src/ports you can see support files
for FreeType, macOS, and Windows GDI font engines. Other font engines can easily
be supported in a like manner.

---

## Does Skia shape text (kerning)?

Shaping is the process that translates a span of Unicode text into a span of
positioned glyphs with the appropriate typefaces.

Skia does not shape text. Skia provides interfaces to draw glyphs, but does not
implement a text shaper. Skia's client's often use
[HarfBuzz](http://www.freedesktop.org/wiki/Software/HarfBuzz/) to generate the
glyphs and their positions, including kerning.

[Here is an example of how to use Skia and HarfBuzz together](https://github.com/aam/skiaex).
In the example, a `SkTypeface` and a `hb_face_t` are created using the same
`mmap()`ed `.ttf` font file. The HarfBuzz face is used to shape unicode text
into a sequence of glyphs and positions, and the `SkTypeface` can then be used
to draw those glyphs.

---

## How do I add drop shadow on text?

<fiddle-embed-sk name='1ff4da09e515087f7011c7caec2e98ae'></fiddle-embed-sk>
