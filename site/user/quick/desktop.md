Desktop
=======

Instructions to get started with Skia on desktop systems (Linux, Mac OS X, or Windows).

1.  [Download Skia](/user/download)

    <!--?prettify lang=sh?-->

        git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
        export PATH="${PWD}/depot_tools:${PATH}"
        git clone 'https://skia.googlesource.com/skia.git'
        cd skia

    (On Windows without git, swap steps 1 and 2)

2.  Install system-specific prerequisites.
    -   [Linux](/user/quick/linux)
    -   [Mac OS X](/user/quick/macos)
    -   [Windows](/user/quick/windows)

3.  Sync dependencies and config.  Build.  Run tests.

    <!--?prettify lang=sh?-->

        python bin/sync-and-gyp
        ninja -C out/Debug
        out/Debug/dm

Gyp Options
-----------

Skia relies on Gyp to generate build files.  Read about
[specifying options for Gyp](/user/tips#gypdefines) to set the
compile-time settings, compiler (e.g. use clang instead of gcc), build systems,
and build directory.

Run unit and correctness tests
------------------------------

[DM](../../dev/testing/testing) ("diamond master") is the Skia test app.

<!--?prettify lang=sh?-->

    ninja -C out/Debug dm
    out/Debug/dm

Run Skia samples
----------------

[SampleApp](../sample/sampleapp) is the Skia sample program.

<!--?prettify lang=sh?-->

    ninja -C out/Debug SampleApp
    out/Debug/SampleApp

Build non-debug binaries
------------------------

The usual mode you want for testing is Debug mode (`SK_DEBUG` is
defined, and debug symbols are included in the binary). If you
would like to build the Release mode:

<!--?prettify lang=sh?-->

    ninja -C out/Release

Performance tests
-----------------

Build and run nanobench (performance tests). In this case, we will
build with the "Release" configuration, since we are running
performance tests.

<!--?prettify lang=sh?-->

    ninja -C out/Release nanobench
    out/Release/nanobench
    out/Release/nanobench --skps .../path/to/*.skp

<!-- TODO(mtklein): document nanobench -->

Keeping up to date
------------------

<!--?prettify lang=sh?-->

    git fetch origin
    git checkout origin/master

Contribute to Skia
------------------

[How to use Git and Git-cl to contribute to Skia](/dev/contrib/submit).
