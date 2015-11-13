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

When running `sync-and-gyp`, the `GYP_DEFINES` environment variable can
be used to change Skia’s compile-time settings, using a
space-separated list of key=value pairs. For example, to disable both
the Skia GPU backend and PDF backends, run it as follows:

<!--?prettify lang=sh?-->

    GYP_DEFINES='skia_gpu=0 skia_pdf=0' python bin/sync-and-gyp
    ninja -C out/Debug

Note: Setting enviroment variables in the Windows CMD.EXE shell [uses a
different syntax](./windows#env).

You can also set environment variables such as `CC`, `CXX`,
`CFLAGS`, or `CPPFLAGS` to control how Skia is compiled. For
example:

<!--?prettify lang=sh?-->

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
