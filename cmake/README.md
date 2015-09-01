CMake build for Skia
====================
This directory contains experiemental CMake build files for Skia.
They are primarily targeted at building Skia as it would be shipped,
not at day-to-day Skia development.

Quickstart
----------
    $ cd skia/cmake
    $ cmake . -G Ninja     # Other CMake generators should work fine.
    $ ninja
    $ ls -l libskia.* example
    $ ./example
    $ open example.png
If that works, you should see "Hello World!" with a green-to-purple gradient.

Currently supported platforms
-----------------------------
  (None.  This is still super experimental.)

Currently maybe-kinda-working platforms
---------------------------------------
  - x86-64 Mac OS X, Ubuntu 15.04

Caveats
-------
  - SkCodec, Skia's new image decoder library, does not yet build.
