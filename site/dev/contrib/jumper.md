Contributing to SkJumper
========================

SkJumper is the execution engine of SkRasterPipeline, a system we've been using
to accelerate CPU-bound work inside Skia, most notably color-space conversions
and color-correct drawing.

(This is where I'd put my link to design document if I had one...)

SkJumper is more annoying to contribute to than most Skia code because of its
offline compilation step.  `src/jumper/build_stages.py` compiles
`src/jumper/SkJumper_stages.cpp` several different ways and parses the object
files it generates into `src/jumper/SkJumper_generated.S` and
`src/jumper/SkJumper_generated_win.S`.  This document is designed to guide you
through this process and ease some of that annoyance.

One-time Setup
--------------

To run `build_stages.py` you need Clang 4.0 and objdump, and probably want
ccache.  It's best that Clang is exactly the same version we typically use (as
of writing 4.0.0) and you'll need objdump to be compiled with support for
x86-64, ARMv7, and ARMv8.

The easiest way to satisfy these contraints is to get your hands on a Mac and
install [Homebrew](https://brew.sh).  Once you have `brew` installed, run this
to get the tools you need:

   $ brew install llvm binutils ccache

Running `build_stages.py`
-------------------------

With your tools installed, try a no-op run of `build_stages.py`:

   $ python src/jumper/build_stages.py path/to/clang-4.0 path/to/gobjdump path/to/ccache
   $ git status

When you run `git status` you should see a bunch of untracked `.o` files
sitting in skia/, and no changes to `src/jumper/SkJumper_generated*.S`.
That's good.  Those object files are the intermediates we parse to produce
the assembly files.  We just leave them around in case you want to look at
them yourself.  If you don't like them, it's safe to just

   $ rm \*.o

If `clang-4.0`, `gobjdump`, and `ccache` are on your path, `build_stages.py`
should find them without you needing to pass them on the command line.

Make A Change
-------------

Let's use the `from_srgb` stage as a little playground to make a real change.
Linearizing sRGB encoded bytes is slow, so let's pretend we've decided to trade
quality for speed, approximating the existing implementation with a simple square.

Open up `SkJumper_stages.cpp` and find the `from_srgb` stage.  It'll look like

    STAGE(from_srgb) {
        ...
    }

Let's replace whatever's there with our fast approximation:

    STAGE(from_srgb) {
        r *= r;
        g *= g;
        b *= b;
    }

When you save and re-run `build_stages.py`, you should now see changes to
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

At this point you can re-build Skia, run DM, compare images, etc. as normal.
Any time you change `SkJumper_stages.cpp`, you need to re-run `build_stages.py`
for those changes to take effect.  Believe me, I'd bake this into our GN build
if I could figure out a way.

Adding a new Stage
------------------

Adding a new stage is a lot like changing an existing stage.  Edit
`SkJumper_stages.cpp`, run `build_stages.py`, build Skia, test, repeat until
correct.

You'll just need to also edit `SkRasterPipeline.h` to add your new stage to the
macro listing all the stages.  The stage name is the handle normal Skia code
uses to refer to the stage abstractly, and the wiring between
`SkRasterPipeline::foo` and `STAGE(foo) { ... }` should work automatically.
