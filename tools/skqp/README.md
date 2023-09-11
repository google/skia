SkQP
====

SkQP (Skia Quality Program) is a component of the Android CTS (Compatablity
Test Suite) that tests an Android device's GPU and OpenGLES & Vulkan drivers
using Skia's existing unit & rendering tests.

See https://skia.org/dev/testing/skqp for pre-build APKs.

How to build and run the SkQP tests
-----------------------------------

1.  Get the dependencies:

    -   You will need Java JDK 8, `git`, and `python`.

    -   Install Chromium's [depot\_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html).  Add it to your `PATH`.

            git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
            export PATH="${PWD}/depot_tools:${PATH}"

    -   Install the [Android NDK](https://developer.android.com/ndk/downloads/).

            ( cd ~; unzip ~/Downloads/android-ndk-*.zip )
            ANDROID_NDK_HOME=$(ls -d ~/android-ndk-*)   # Or wherever you installed the Android NDK.

    -   Install the [Android SDK](https://developer.android.com/studio/#command-tools).
        Set the `ANDROID_HOME` environment variable.

            mkdir ~/android-sdk
            ( cd ~/android-sdk; unzip ~/Downloads/sdk-tools-*.zip )
            yes | ~/android-sdk/tools/bin/sdkmanager --licenses
            export ANDROID_HOME=~/android-sdk  # Or wherever you installed the Android SDK.

        Put `adb` in your `PATH`.

            export PATH="${PATH}:${ANDROID_HOME}/platform-tools"

2.  Get the right version of Skia:

        git clone https://skia.googlesource.com/skia.git
        cd skia
        git checkout origin/skqp/dev  # or whatever release tag you need

3.  Build the APK:

        tools/git-sync-deps
        tools/skqp/make_universal_apk

4.  Build, install, and run.

        adb install -r out/skqp/skqp-universal-debug.apk
        adb logcat -c
        adb shell am instrument -w org.skia.skqp

5.  Monitor the output with:

        adb logcat TestRunner org.skia.skqp skia "*:S"

    Note the test's output path on the device.  It will look something like this:

        01-23 15:22:12.688 27158 27173 I org.skia.skqp:
        output written to "/storage/emulated/0/Android/data/org.skia.skqp/files/output"

6.  Retrieve and view the report with:

        OUTPUT_LOCATION="/storage/emulated/0/Android/data/org.skia.skqp/files/output"
        adb pull $OUTPUT_LOCATION /tmp/
        bin/sysopen /tmp/output/skqp_report/report.html

Running a single test
---------------------

To run a single test, for example `gles_aarectmodes`:

    adb shell am instrument -e class 'org.skia.skqp.SkQPRunner#gles_aarectmodes' -w org.skia.skqp

Unit tests can be run with the `unitTest_` prefix:

    adb shell am instrument -e class 'org.skia.skqp.SkQPRunner#unitTest_GrSurface -w org.skia.skqp

Run as a non-APK executable
---------------------------

1.  Follow steps 1-3 as above.

2.  Build the SkQP program, load files on the device, and run skqp:

        ninja -C out/skqp/arm skqp
        python tools/skqp/run_skqp_exe out/skqp/arm
