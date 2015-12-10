Android
=======

Prerequisites
-------------

_Currently we only support building Skia for Android on a Linux or Mac host! In addition,
 we only use the Mac build for local development. All shipping variants are compiled on
 Linux for performance reasons._

The following libraries/utilities are required in addition to those needed for a standard skia checkout:

  * The Android SDK: http://developer.android.com/sdk/

Check out the source code
-------------------------

Follow the instructions [here](../download) for downloading the Skia source. 

Inside your Skia checkout, `platform_tools/android` contains the Android setup
scripts, Android specific dependencies, and the Android Sample App.

You may need to [install other dependencies](./linux#prerequisites):

    tools/install_dependencies.sh

Setup the Android SDK
---------------------

To finish setting up the Android SDK you need to download use the SDK to
download the appropriate API level.  To do this simply go to the directory
where you installed the SDK and run the following commands

    # You may want to add this export to your shell's .bash_profile or .profile
    export ANDROID_SDK_ROOT=/path/to/android/sdk

    $ ANDROID_SDK_ROOT/tools/android update sdk --no-ui --filter android-19

From here you will need to type 'y' to approve the license agreement and that
is all.  You will then have downloaded the SDK for API level 19 (Android 4.4
KitKat) which will be used to build the Skia SampleApp.  You can download as
many other Android add-ons or APIs as you want, but you only are required to
have this one in order to complete the Skia build process.

Setup Environment for Android
-----------------------------

The Android build needs to set up some specific variables needed by both GYP
and Make. We make this setup easy for developers by encapsulating all the
details into a custom script that acts as a replacement for make.

Custom Android Build Script
---------------------------

The android_ninja script is a wrapper for the ninja command (provided by
depot_tools) and is specifically designed to work with the Skia's build
system. To use the script you need to call it from Skia's trunk directory with
the -d option plus any of the options or arguments you would normally pass to
ninja (see descriptions of some of the other flags here).

    export ANDROID_SDK_ROOT=/path/to/android/sdk
    export ANDROID_HOME=/path/to/android/sdk
    export PATH=$PATH:/path/to/depot_tools

    cd skia
    ./platform_tools/android/bin/android_ninja -d nexus_10 # or nexus_7, galaxy_nexus, etc...

The -d option enables the build system to target the build to a specific
architecture such as MIPS (generic), x86 (generic) and ARM (generic and device
specific flavors for Nexus devices). This in turn allows Skia to take
advantage of specific device optimizations (e.g. NEON instructions).

Generate build file from GYP
----------------------------

We use the open-source gyp tool to generate build files from our
multi-platform "gyp" files. While most other platforms enable you to
regenerate these files using `./gyp_skia` or `bin/sync-and-gyp` it is
recommended that you do NOT do this for Android.  Instead you can rely
on it being run automatically by android_ninja.

Faster rebuilds
---------------

You can use ccache to improve the speed of rebuilding:

    # You may want to add this export to your shell's .bash_profile or .profile
    export ANDROID_MAKE_CCACHE=[ccache]

Build and run executables on the device
---------------------------------------

The build system packages the Skia executables as shared libraries.  As such,
in order to run any executable on the device you must install the library and
a launcher executable on your device.  To assist in this process there is a
script called `android_run_skia` that is located in the
`platform_tools/android/bin` directory.

Run correctness tests
---------------------

First build the app and then run it on an attached device:

    ./platform_tools/android/bin/android_ninja [-d device_id] dm

    # uploads dm binary and resources and runs dm on the attached device
    ./platform_tools/android/bin/android_run_skia dm --resourcePath /data/local/tmp/skia/resources/

Run performance tests
---------------------

Since nanobench tests performance, it usually makes more sense to run it in
Release mode.

    BUILDTYPE=Release ./platform_tools/android/bin/android_ninja [-d device_id] nanobench

    # uploads and runs the nanobench binary on the attached device
    ./platform_tools/android/bin/android_run_skia --release nanobench

If you pass nanobench SKP files, it will benchmark them too.

    ./platform_tools/android/bin/[linux/mac]/adb push ../skp <dst> # <dst> is dir on device

Finally to run the executable there are two approaches. The simplest of the
two run the app on the device like you would do for gm or tests, however this
approach will also produce the noisiest results.

    # <input> is file/dir on device
    ./platform_tools/android/bin/android_run_skia --release nanobench --skps <input>

Build and run SampleApp
-----------------------

The SampleApp on Android provides a simple UI for viewing sample slides and gm images.

    BUILDTYPE=Debug ./platform_tools/android/bin/android_ninja -d $TARGET_DEVICE SampleApp_APK

Then, install the app onto the device:

    ./platform_tools/android/bin/android_install_app

Finally to run the application you can either navigate to the Skia Samples
application using the application launcher on your device or from the command
line.  The command line option allows you to pass additional details to the
application (similar to other operating system) that specify where to find
skp files and other resources.

    ./platform_tools/android/bin/android_launch_app --resourcePath /data/local/tmp/resources

By default if no additional parameters are specified the app will use the default
parameters...

    --resourcePath /data/local/tmp/skia_resoures 
    --pictureDir /data/local/tmp/skia_skp


Android Studio Support
-----------------------

You can also build and run SampleApp (and some other experimental apps) using Android
Studio.  To create the project either select "import project" from the quickstart
screen or use File -> Open.  In both cases you'll need to select ./platform_tools/android/apps
as the root directory of your project.

Finally to be able to build within Android studio it needs to know the path to 
ninja so you will need to add a properties file and populate it with the path
to depot_tools.  The syntax and location of that file is...
 
    # 
    # file location: ./platform_tools/android/apps/gradle.properties
    #
    depot_tools.dir=<path_to_depot_tools>
    
That should be all the setup you need.  You should now be able to build and deploy
SampleApp on ARM, Intel, and MIPS devices.


Build tools
-----------

The Android platform does not support skdiff at this time.

Clean up all generated files
----------------------------

    make clean

Debugging on Android
--------------------

We support 2 modes of debugging on Android using GDB wrapper scripts. These
scripts start a gdbserver instance on the device and then enter an interactive
GDB client shell on your host. All necessary symbol files should
be pulled from the device and placed into a temporary folder (android_gdb_tmp).

Note: The debugging scripts do not build the app - you'll have to do that first.

    # COMMAND LINE APPS
    # include additional arguments in quotes (e.g. "dm --nopdf")
    ./platform_tools/android/bin/android_gdb_native dm
    
    # SAMPLE APP
    # make sure you've installed the app on the device first
    ./platform_tools/android/bin/android_gdb_app [-d device_id]

When the gdb client is ready, insert a breakpoint, and continue to let the
program resume execution.
