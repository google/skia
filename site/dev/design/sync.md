sync
====

[`sync`](https://skia.googlesource.com/skia.git/+/master/bin/sync)
is a Python program that wraps `gclient sync`.  Motivations for using it:

-  Written in Python, so it will work on all platforms.

-  Sets up gclient better than `gclient config`, which has been broken.

-  Checks to see if the `DEPS` file has changed since it last ran
   `gclient sync`.  If not, it skips that step.

-  Since running `sync` is fast when it can do nothing, it is
   easy to do before every recompile of Skia.  This is a good habit.
