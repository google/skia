iOS
===

The following has been tested on MacOS Yosemite with Xcode version 6.3.

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

    # Create the project files.
    GYP_DEFINES="skia_os='ios' skia_arch_type='arm' armv7=1 arm_neon=0" python bin/sync-and-gyp
    # Build and run SampleApp.
    xed out/gyp/SampleApp.xcodeproj # opens the SampleApp project in Xcode

Prerequisites
-------------

Make sure the following have been installed:

  * XCode (Apple's development environment): required
    * publicly available at http://developer.apple.com/xcode/
    * add the optional Unix Tools to the install so you get the make command line tool.
  * Chromium depot_tools: required to download the source and dependencies
    * http://www.chromium.org/developers/how-tos/depottools
  * You will need an Apple developer account if you wish to run on an iOS device.
  * A tool such as [ios-deploy](https://github.com/phonegap/ios-deploy) is also useful for pulling output from an iOS device.

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

Or you can set it to `xcode` alone, if you like.

You can then generate the Xcode projects by running:

    GYP_DEFINES="skia_os='ios' skia_arch_type='arm' armv7=1 arm_neon=0" python bin/sync-and-gyp

Alternatively, you can do:

    export GYP_DEFINES="skia_os='ios' skia_arch_type='arm' armv7=1 arm_neon=0"
    python bin/sync-and-gyp

Build and run tests
-------------------

The 'dm' test program is wrapped in an app called iOSShell. The project for iOSShell is at out/gyp/iOSShell.xcodeproj. 
Running this app with the flag '--dm' will run unit tests and golden master images. Other arguments to the standard 'dm'
test program can also be passed in.

To launch the iOS app on a device from the command line you can use a tool such as [ios-deploy](https://github.com/phonegap/ios-deploy):

    xcodebuild -project out/gyp/iOSShell.xcodeproj -configuration Debug
    ios-deploy --bundle xcodebuild/Debug-iphoneos/iOSShell.app -I -d --args "--dm <dm_args>"

The usual mode you want for testing is Debug mode (SK_DEBUG is defined, and
debug symbols are included in the binary). If you would like to build the
Release version instead:

    xcodebuild -project out/gyp/iOSShell.xcodeproj -configuration Release
    ios-deploy --bundle xcodebuild/Release-iphoneos/iOSShell.app -I -d --args "--dm <dm_args>"

Build and run nanobench (performance tests)
-------------------------------------------

The 'nanobench' test program is also wrapped in iOSShell.app. Passing in the flag '--nanobench' will run these tests.

Here's an example of running nanobench from the command line. We will build with the "Release" configuration, since we are running performance tests.

    xcodebuild --project out/gyp/iOSShell.xcodeproj -configuration Release
    ios-deploy --bundle xcodebuild/Release-iphoneos/iOSShell.app -I -d --args "--nanobench <nanobench_args>"

Build and run SampleApp in the XCode IDE
----------------------------------------

  * Run `sync-and-gyp` as described above.
  * In the Finder, navigate to $SKIA_INSTALLDIR/trunk/out/gyp
  * Double-click SampleApp.xcodeproj ; this will launch XCode and open the SampleApp project
  * Make sure the SampleApp target is selected, and choose an iOS device to run on
  * Click the “Build and Run” button in the top toolbar
  * Once the build is complete, launching the app will display a window with lots of shaded text examples. On the upper left there is a drop down
menu that allows you to cycle through different test pages. On the upper right there is a dialog with a set of options, including different
rendering methods for each test page.

Provisioning
------------

To run the Skia apps on an iOS device rather than using the simulator, you will need a developer account and a provisioning profile. See
[Launching Your App on Devices](https://developer.apple.com/library/ios/documentation/IDEs/Conceptual/AppDistributionGuide/LaunchingYourApponDevices/LaunchingYourApponDevices.html) for more information.

Managing App Data
-----------------
By default, the iOS apps will look for resource files in the Documents/resources folder of the app and write any output files to Documents/. To upload resources
so that the app can read them you can use a tool such as [ios-deploy](https://github.com/phonegap/ios-deploy). For example:

    ios-deploy --bundle_id 'com.google.SkiaSampleApp' --upload resources/baby_tux.png --to Documents/resources/baby_tux.png

You can use the same tool to download log files and golden master (GM) images:

    ios-deploy --bundle_id 'com.google.iOSShell' --download=/Documents --to ./my_download_location

Alternatively, you can put resources and other files in the bundle of the application. In this case, you'll need to run the app with the option '--resourcePath .'
