Mac OS X
========

Quickstart
----------

1.  Install [XCode](http://developer.apple.com/xcode/).

2.  Install depot tools.

    <!--?prettify lang=sh?-->

        git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
        export PATH="${PWD}/depot_tools:${PATH}"

3.  Get Skia.

    <!--?prettify lang=sh?-->

        git clone 'https://skia.googlesource.com/skia'
        cd skia

4.  Build.

    <!--?prettify lang=sh?-->

        bin/sync-and-gyp && ninja -C out/Debug

5.  Run DM (the Skia test app) and SampleApp.

    <!--?prettify lang=sh?-->

        out/Debug/dm
        open out/Debug/SampleApp.app

Prerequisites
-------------

Make sure the following have been installed:

  * XCode (Apple's development environment): required
    * publicly available at http://developer.apple.com/xcode/
    * add the optional Unix Tools to the install so you get the make command line tool.
  * Chromium depot_tools: required to download the source and dependencies
    * http://www.chromium.org/developers/how-tos/depottools

Check out the source code
-------------------------

See the instructions [here](../download).

Generate XCode projects
-----------------------

We use the open-source gyp tool to generate XCode projects (and analogous
build scripts on other platforms) from our multiplatform "gyp" files.

Before building, make sure that gyp knows to create an XCode project or ninja
build files. If you leave GYP_GENERATORS undefined it will assume the
following default:

    GYP_GENERATORS="ninja,xcode"

Or you can set it to `ninja` or `xcode` alone, if you like.

You can then generate the Xcode projects and ninja build files by running:

    ./gyp_skia

Build and run tests from the command line
-----------------------------------------

    ninja -C out/Debug dm
    out/Debug/dm

The usual mode you want for testing is Debug mode (SK_DEBUG is defined, and
debug symbols are included in the binary). If you would like to build the
Release version instead:

    ninja -C out/Release dm
    out/Release/dm

Build and run nanobench (performance tests)
-------------------------------------------

In this case, we will build with the "Release" configuration, since we are running performance tests.

    ninja -C out/Release nanobench
    out/Release/nanobench [ --skps path/to/*.skp ]

Build and run SampleApp in the XCode IDE
----------------------------------------

  * Run gyp_skia as described above.
  * In the Finder, navigate to $SKIA_INSTALLDIR/trunk/out/gyp
  * Double-click SampleApp.xcodeproj ; this will launch XCode and open the SampleApp project
  * Click the “Build and Run” button in the top toolbar
  * Once the build is complete, you should see a window with lots of shaded text examples. To move through the sample app, use the following keypresses:
    * right- and left-arrow keys: cycle through different test pages
    * 'D' key: cycle through rendering methods for each test page
    * other keys are defined in SampleApp.cpp’s SampleWindow::onHandleKey() and SampleWindow::onHandleChar() methods
