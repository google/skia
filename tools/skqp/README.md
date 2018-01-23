
SkQP
====

**Motivation**: Test an Android device's GPU and OpenGLES & Vulkan drivers with
Skia and Skia's existing unit & rendering tests.

How To Use SkQP on your Android device:

1.  To build SkQP you need to install the
    [Android NDK](https://developer.android.com/ndk/).

2.  [Checkout depot\_tools and Skia](https://skia.org/user/download),
    then go to Skia's source directory:

        export PATH="${DEPOT_TOOLS_PATH}:$PATH"
        cd $SKIA_SOURCE_DIRECTORY

3.  Configure and build Skia for your device's architecture:

        arch='arm64'  # Also valid: 'arm', 'x68', 'x64'
        android_ndk="${HOME}/android-ndk"  # Or wherever you installed the NDK.

        tools/skqp/generate_gn_args.sh out/${arch}-rel "$android_ndk" $arch
        tools/git-sync-deps
        bin/gn gen out/${arch}-rel
        ninja -C out/${arch}-rel skqp_lib

4.  Download meta.json from [https://goo.gl/jBw3Dd](https://goo.gl/jBw3Dd) .
    This is the data used to build the validation model.

5.  Generate the validation model data:

        tools/skqp/make_model.sh ~/Downloads/meta.json

Run as an executable
--------------------

1.  Build the SkQP program, load files on the device, and run skqp:

        ninja -C out/${arch}-rel skqp
        adb shell "cd /data/local/tmp; rm -rf skqp_assets report"
        adb push platform_tools/android/apps/skqp/src/main/assets \
            /data/local/tmp/skqp_assets
        adb push out/${arch}-rel/skqp /data/local/tmp/
        adb shell "cd /data/local/tmp; ./skqp skqp_assets report"

2.  Get the error report if there are errors:

        adb pull /data/local/tmp/report /tmp/
        tools/skqp/sysopen.py /tmp/report/report.html

Run as an APK
-------------

0.  Install the [Android SDK](https://developer.android.com/studio/#command-tools).

        mkdir ~/android-sdk
        ( cd ~/android-sdk; unzip ~/Downloads/sdk-tools-*.zip )
        yes | ~/android-sdk/tools/bin/sdkmanager --licenses
        export ANDROID_HOME=~/android-sdk

1.  Build the skqp.apk, load it on the device, and run the tests

        platform_tools/android/bin/android_build_app -C out/${arch}-rel skqp
        adb install -r out/${arch}-rel/skqp.apk
        adb logcat -c
        adb shell am instrument -w \
            org.skia.skqp/android.support.test.runner.AndroidJUnitRunner

2.  Find out where the report went (and look for Skia errors):

        adb logcat -d org.skia.skqp skia "*:S"

3.  Retrieve and view the report if there are any errors:

        adb pull /storage/emulated/0/Android/data/org.skia.skqp/files/output /tmp/
        tools/skqp/sysopen.py /tmp/output/skqp_report/report.html



