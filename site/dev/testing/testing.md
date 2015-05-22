Correctness Testing
===================

Skia correctness testing is primarily served by a tool named DM.
This is a quickstart to building and running DM.

~~~
$ ./gyp_skia
$ ninja -C out/Debug dm
$ out/Debug/dm -v -w dm_output
~~~

When you run this, you may notice your CPU peg to 100% for a while, then taper
off to 1 or 2 active cores as the run finishes.  This is intentional.  DM is
very multithreaded, but some of the work, particularly GPU-backed work, is
still forced to run on a single thread.  You can use `--threads N` to limit DM to
N threads if you like.  This can sometimes be helpful on machines that have
relatively more CPU available than RAM.

As DM runs, you ought to see a giant spew of output that looks something like this.
~~~
Skipping nonrendering: Don't understand 'nonrendering'.
Skipping angle: Don't understand 'angle'.
Skipping nvprmsaa4: Could not create a surface.
492 srcs * 3 sinks + 382 tests == 1858 tasks

(  25MB  1857) 1.36ms   8888 image mandrill_132x132_12x12.astc-5-subsets
(  25MB  1856) 1.41ms   8888 image mandrill_132x132_6x6.astc-5-subsets
(  25MB  1855) 1.35ms   8888 image mandrill_132x130_6x5.astc-5-subsets
(  25MB  1854) 1.41ms   8888 image mandrill_132x130_12x10.astc-5-subsets
(  25MB  1853) 151µs    8888 image mandrill_130x132_10x6.astc-5-subsets
(  25MB  1852) 154µs    8888 image mandrill_130x130_5x5.astc-5-subsets
                                  ...
( 748MB     5) 9.43ms   unit test GLInterfaceValidation
( 748MB     4) 30.3ms   unit test HalfFloatTextureTest
( 748MB     3) 31.2ms   unit test FloatingPointTextureTest
( 748MB     2) 32.9ms   unit test DeferredCanvas_GPU
( 748MB     1) 49.4ms   unit test ClipCache
( 748MB     0) 37.2ms   unit test Blur
~~~
Do not panic.

As you become more familiar with DM, this spew may be a bit annoying. If you
remove -v from the command line, DM will spin its progress on a single line
rather than print a new line for each status update.

Don't worry about the "Skipping something: Here's why." lines at startup.  DM
supports many test configurations, which are not all appropriate for all
machines.  These lines are a sort of FYI, mostly in case DM can't run some
configuration you might be expecting it to run.

The next line is an overview of the work DM is about to do.
~~~
492 srcs * 3 sinks + 382 tests == 1858 tasks
~~~

DM has found 382 unit tests (code linked in from tests/), and 492 other drawing
sources.  These drawing sources may be GM integration tests (code linked in
from gm/), image files (from `--images`, which defaults to "resources") or .skp
files (from `--skps`, which defaults to "skps").  You can control the types of
sources DM will use with `--src` (default, "tests gm image skp").

DM has found 3 usable ways to draw those 492 sources.  This is controlled by
`--config`, which today defaults to "565 8888 gpu nonrendering angle nvprmsaa4".
DM has skipped nonrendering, angle, and nvprmssa4, leaving three usable configs:
565, 8888, and gpu.  These three name different ways to draw using Skia:

  -    565:  draw using the software backend into a 16-bit RGB bitmap
  -    8888: draw using the software backend into a 32-bit RGBA bitmap
  -    gpu:  draw using the GPU backend (Ganesh) into a 32-bit RGBA bitmap

Sometimes DM calls these configs, sometimes sinks.  Sorry.  There are many
possible configs but generally we pay most attention to 8888 and gpu.

DM always tries to draw all sources into all sinks, which is why we multiply
492 by 3.  The unit tests don't really fit into this source-sink model, so they
stand alone.  A couple thousand tasks is pretty normal.  Let's look at the
status line for one of those tasks.
~~~
(  25MB  1857) 1.36ms   8888 image mandrill_132x132_12x12.astc-5-subsets
~~~

This status line tells us several things.

First, it tells us that at the time we wrote the status line, the maximum
amount of memory DM had ever used was 25MB.  Note this is a high water mark,
not the current memory usage.  This is mostly useful for us to track on our
buildbots, some of which run perilously close to the system memory limit.

Next, the status line tells us that there are 1857 unfinished tasks, either
currently running or waiting to run.  We generally run one task per hardware
thread available, so on a typical laptop there are probably 4 or 8 running at
once.  Sometimes the counts appear to show up out of order, particularly at DM
startup; it's harmless, and doesn't affect the correctness of the run.

Next, we see this task took 1.36 milliseconds to run.  Generally, the precision
of this timer is around 1 microsecond.  The time is purely there for
informational purposes, to make it easier for us to find slow tests.

Finally we see the configuration and name of the test we ran.  We drew the test
"mandrill_132x132_12x12.astc-5-subsets", which is an "image" source, into an
"8888" sink.

When DM finishes running, you should find a directory with file named dm.json,
and some nested directories filled with lots of images.
~~~
$ ls dm_output
565     8888    dm.json gpu

$ find dm_output -name '*.png'
dm_output/565/gm/3x3bitmaprect.png
dm_output/565/gm/aaclip.png
dm_output/565/gm/aarectmodes.png
dm_output/565/gm/alphagradients.png
dm_output/565/gm/arcofzorro.png
dm_output/565/gm/arithmode.png
dm_output/565/gm/astcbitmap.png
dm_output/565/gm/bezier_conic_effects.png
dm_output/565/gm/bezier_cubic_effects.png
dm_output/565/gm/bezier_quad_effects.png
                ...
~~~

The directories are nested first by sink type (`--config`), then by source type (`--src`).
The image from the task we just looked at, "8888 image mandrill_132x132_12x12.astc-5-subsets",
can be found at dm_output/8888/image/mandrill_132x132_12x12.astc-5-subsets.png.

dm.json is used by our automated testing system, so you can ignore it if you
like.  It contains a listing of each test run and a checksum of the image
generated for that run.

### Detail <a name="digests"></a>
Boring technical detail: The checksum is not a checksum of the
.png file, but rather a checksum of the raw pixels used to create that .png.
That means it is possible for two different configurations to produce
the same exact .png, but have their checksums differ.

Unit tests don't generally output anything but a status update when they pass.
If a test fails, DM will print out its assertion failures, both at the time
they happen and then again all together after everything is done running.
These failures are also included in the dm.json file.

DM has a simple facility to compare against the results of a previous run:
~~~
$ ./gyp_skia
$ ninja -C out/Debug dm
$ out/Debug/dm -w good

   (do some work)

$ ./gyp_skia
$ ninja -C out/Debug dm
$ out/Debug/dm -r good -w bad
~~~
When using `-r`, DM will display a failure for any test that didn't produce the
same image as the `good` run.

For anything fancier, I suggest using skdiff:
~~~
$ ./gyp_skia
$ ninja -C out/Debug dm
$ out/Debug/dm -w good

   (do some work)

$ ./gyp_skia
$ ninja -C out/Debug dm
$ out/Debug/dm -w bad

$ ninja -C out/Debug skdiff
$ mkdir diff
$ out/Debug/skdiff good bad diff

  (open diff/index.html in your web browser)
~~~

That's the basics of DM.  DM supports many other modes and flags.  Here are a
few examples you might find handy.
~~~
$ out/Debug/dm --help        # Print all flags, their defaults, and a brief explanation of each.
$ out/Debug/dm --src tests   # Run only unit tests.
$ out/Debug/dm --nocpu       # Test only GPU-backed work.
$ out/Debug/dm --nogpu       # Test only CPU-backed work.
$ out/Debug/dm --match blur  # Run only work with "blur" in its name.
$ out/Debug/dm --dryRun      # Don't really do anything, just print out what we'd do.
~~~
