Contributing to SkJumper
========================

SkJumper is the execution engine of SkRasterPipeline, a system we've been using
to accelerate CPU-bound work inside Skia, most notably color-space conversions
and color-correct drawing.

(This is where I'd put my link to design document if I had one...)

SkJumper is more annoying to contribute to than most Skia code because of its
offline compilation step.  You'll need particular tools installed on your
machine and to tell GN about them.  This document is designed to guide you
through this process and ease some of that annoyance.

One-time Setup
--------------

To generate stage code you need Clang 4.0, objdump, and ccache.  It's best that
Clang is exactly the same version we typically use (as of writing 4.0.0) and
you'll need objdump to be compiled with support for x86-64, ARMv7, and ARMv8.

The easiest way to satisfy these contraints is to get your hands on a Mac and
install [Homebrew](https://brew.sh).  Once you have `brew` installed, run this
to get the tools you need:

<!--?prettify lang=sh?-->

    brew install llvm binutils ccache

Setting up GN
-------------------------

With your tools installed, tell GN about them

    skia_jumper_clang = path/to/clang-4.0
    skia_jumper_objdump = path/to/gobjdump
    skia_jumper_ccache = path/to/ccache

then regenerate and build as normal.

If you look in your GN out directory, you should now see a bunch of `.o` files,
and `git status` should show no changes to `src/jumper/SkJumper_generated*.S`.
That's good.  Those object files are the intermediates we parse to produce
the assembly files.  We just leave them around in case you want to look at
them yourself.

Make A Change
-------------

Let's use the `from_srgb` stage as a little playground to make a real change.
Linearizing sRGB encoded bytes is slow, so let's pretend we've decided to trade
quality for speed, approximating the existing implementation with a simple square.

Open up `SkJumper_stages.cpp` and find the `from_srgb` stage.  It'll look like

<!--?prettify lang=cc?-->

    STAGE(from_srgb) {
        r = from_srgb(r);
        g = from_srgb(g);
        b = from_srgb(b);
    }

Let's replace whatever's there with our fast approximation:

<!--?prettify lang=cc?-->

    STAGE(from_srgb) {
        r *= r;
        g *= g;
        b *= b;
    }

When you save and re-Ninja, you should now see changes to
`src/jumper/SkJumper_generated.S` and `src/jumper/SkJumper_generated_win.S`.
If you can't read assembly, no big deal.  If you can, run `git diff`.  You
should see the various `sk_from_srgb_*` functions get dramatically simpler,
something like three multiplies and a couple other bookkeeping instructions.

It's not unusual for isolated changes in one stage to cause seemingly unrelated
changes in another.  When adding or removing any code you'll usually see all
the comments in branch instructions change a little bit, but the actual
instruction on the left won't change.  When adding or removing uses of
constants, you'll often see both the comment and instruction on the left change
for other loads of constants from memory, especially on x86-64.  You'll also
see some code that looks like garbage change; those are the constants.  If
any of this worries you, please do go running to someone who knows more for
help, but odds are everything is fine.

At this point things should just be business as usual.  Any time you change
`SkJumper_stages.cpp`, Ninja ought to notice and regenerate the assembly files.

Adding a new Stage
------------------

Adding a new stage is a lot like changing an existing stage.  Edit
`SkJumper_stages.cpp`, build Skia, test, repeat until correct.

You'll just need to also edit `SkRasterPipeline.h` to add your new stage to the
macro listing all the stages.  The stage name is the handle normal Skia code
uses to refer to the stage abstractly, and the wiring between
`SkRasterPipeline::foo` and `STAGE(foo) { ... }` should work automatically.
