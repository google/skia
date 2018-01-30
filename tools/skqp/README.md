
SkQP
====

How to run the SkQP tests
-------------------------

1.  Install Chromium's depot\_tools

        git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
        export PATH="${PWD}/depot_tools:${PATH}"

2.  Install the [Android NDK](https://developer.android.com/ndk/downloads/).

        cd ~
        unzip ~/Downloads/android-ndk-*.zip
        ANDROID_NDK=~/android-ndk-*        # Or wherever you installed the Android NDK.

3.  Install the [Android SDK](https://developer.android.com/studio/#command-tools)

        mkdir ~/android-sdk
        ( cd ~/android-sdk; unzip ~/Downloads/sdk-tools-*.zip )
        yes | ~/android-sdk/tools/bin/sdkmanager --licenses
        export ANDROID_HOME=~/android-sdk  # Or wherever you installed the Android SDK.

    Put `adb` in your `PATH`.

4.  Get the right version of Skia:

        git clone https://skia.googlesource.com/skia.git
        cd skia
        git checkout origin/skqp/dev  # or whatever release tag you need

5.  Download dependencies, the model, and configure the build.  (If you want to
    test another architecture, replace `arm` with `x86`, `x64`, or `arm64`.)

        python tools/skqp/download_model
        python tools/skqp/setup_resources
        python tools/git-sync-deps
        python tools/skqp/generate_gn_args out/skqp-arm "$ANDROID_NDK" arm
        bin/gn gen out/skqp-arm

6.  Build, install, and run.

        platform_tools/android/bin/android_build_app -C out/skqp-arm skqp
        adb install -r out/skqp-arm/skqp.apk
        adb logcat -c
        adb shell am instrument -w org.skia.skqp/android.support.test.runner.AndroidJUnitRunner

7.  Monitor the output with:

        adb logcat org.skia.skqp skia "*:S"

    Note the test's output path on the device.  It will look something like this:

        01-23 15:22:12.688 27158 27173 I org.skia.skqp:
        output written to "/storage/emulated/0/Android/data/org.skia.skqp/files/output"

8.  Retrieve and view the report with:

        OUTPUT_LOCATION="/storage/emulated/0/Android/data/org.skia.skqp/files/output"
        adb pull $OUTPUT_LOCATION /tmp/
        tools/skqp/sysopen.py /tmp/output/skqp_report/report.html

Run as a non-APK executable
---------------------------

1.  Follow steps 1-5 as above.

2.  Build the SkQP program, load files on the device, and run skqp:

        ninja -C out/skqp-arm skqp
        python tools/skqp/run_skqp_exe out/skqp-arm

