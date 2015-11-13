Mac OS X
========

Quickstart
----------

First, install [XCode](https://developer.apple.com/xcode/). 

<!--?prettify lang=sh?-->

    # Install depot tools.
    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
    export PATH="${PWD}/depot_tools:${PATH}"

    # Get Skia.
    git clone 'https://skia.googlesource.com/skia'
    cd skia

    # Build.
    bin/sync-and-gyp
    ninja -C out/Debug

    # Run DM (the Skia test app) and SampleApp.
    out/Debug/dm
    out/Debug/SampleApp

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

Follow the instructions [here](../download) for downloading the Skia source.

Generate XCode projects
-----------------------

We use the open-source gyp tool to generate XCode projects (and analogous
build scripts on other platforms) from our multiplatform "gyp" files.

Two Gyp generators are used on Mac OS:

*   `ninja` - Run ninja yourself, without XCode project files,

*   `xcode` - Generate a XCode project

To choose which ones to use, set the `GYP_GENERATORS` environment
variable to a comma-delimited list of generators before running
sync-and-gyp. The default value for `GYP_GENERATORS` is
`ninja,xcode`.  For example to enable



Before building, make sure that gyp knows to create an XCode project or ninja
build files. If you leave GYP_GENERATORS undefined it will assume the
following default:

    GYP_GENERATORS="ninja,xcode"

Or you can set it to `ninja` or `xcode` alone, if you like.

You can then generate the Xcode projects and ninja build files by running:

    python bin/sync-and-gyp

Build and run tests from the command line
-----------------------------------------

    ninja -C out/Debug dm
    out/Debug/dm


Build and run SampleApp in the XCode IDE
----------------------------------------

  * Run `sync-and-gyp` as described above.
  * In the Finder, navigate to `$SKIA_INSTALLDIR/trunk/out/gyp`
  * Double-click SampleApp.xcodeproj ; this will launch XCode and open the SampleApp project
  * Click the “Build and Run” button in the top toolbar
  * Once the build is complete, you should see a window with lots of shaded text examples. To move through the sample app, use the following keypresses:
    * right- and left-arrow keys: cycle through different test pages
    * 'D' key: cycle through rendering methods for each test page
    * other keys are defined in SampleApp.cpp’s SampleWindow::onHandleKey() and SampleWindow::onHandleChar() methods
