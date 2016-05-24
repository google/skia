CMake build for Skia
====================
This directory contains experiemental CMake build files for Skia.
They are primarily targeted at building Skia as it would be shipped,
not at day-to-day Skia development.

Quickstart
----------

<!--?prettify lang=sh?-->

    git clone https://skia.googlesource.com/skia.git
    cd skia/cmake
    cmake . -G Ninja     # Other CMake generators should work fine.
    ninja
    ls -l libskia.* example
    ./example
    open example.png || xdg-open example.png

If that works, you should see "Hello World!" with a green-to-purple gradient.

Currently supported platforms
-----------------------------
  (None.  This is still super experimental.)

Currently maybe-kinda-working platforms
---------------------------------------
  - x86-64 Mac OS X
  - x86-64 Ubuntu 15.04
  - x86-64 Windows 10, with extra caveats:
      * Compiles against DirectWrite, not GDI, for fonts
      * Configure with `cmake . -G "Visual Studio 14 2015"` .
      * Compile with `cmake --build . --config Release` .
      * Still has too many warnings.
      * Poorly tested as yet.

