sync-and-gyp
============

[`sync-and-gyp`](https://skia.googlesource.com/skia.git/+/master/bin/sync-and-gyp)
is a Python program that wraps `gclient sync` and `gyp_skia`.
Motivations for using it:

-  Fewer steps to configure and compile Skia and Skia's dependencies.
   This makes documentation cleaner, too.

-  Written in Python, so it will work on all platforms.  Python is
   already necessary for gyp.

-  Sets up gclient better than `gclient config`, which has been broken.

-  Checks to see if the `DEPS` file has changed since it last ran
   `gclient sync`.  If not, it skips that step.

-  Checks to see if gyp needs to be re-run (it checks environment
   variables and changed or added files); if not, it skips running
   `gyp_skia`.

-  Since running `sync-and-gyp` is fast when it can do nothing, it is
   easy to do before every recompile of Skia.  This is a good habit.
