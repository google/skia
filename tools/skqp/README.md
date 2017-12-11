
SkQP
====

**Motivation**: Test an Android device’s GPU and OpenGLES & Vulkan drivers with
Skia and Skia’s existing unit & rendering tests.

How To Use SkQP on your Android device:

1.  To build SkQP you need to install the
    [Android NDK](https://developer.android.com/ndk/).

2.  Checkout Skia, then go to the source directory:

        cd $SKIA_SOURCE_DIRECTORY

3.  Configure and build Skia for your device's architecture:

        arch='arm64'  # Also valid: 'arm', 'x68', 'x64'
        android_ndk="${HOME}/ndk"  # Or wherever you installed the NDK.
        mkdir -p out/${arch}-rel
        cat > out/${arch}-rel/args.gn << EOF
            ndk = "$android_ndk"
            ndk_api = 24
            target_cpu = "$arch"
            skia_embed_resources = true
            is_debug = false
        EOF
        tools/git-sync-deps
        bin/gn gen out/${arch}-rel
        ninja -C out/${arch}-rel skqp_lib

4.  Download meta.json from [https://goo.gl/jBw3Dd](https://goo.gl/jBw3Dd) .
    This is the data used to build the validation model.

5.  Generate the validation model data:

        rm -rf platform_tools/android/apps/skqp/src/main/assets/gmkb
        go get go.skia.org/infra/golden/go/search
        go run tools/skqp/make_gmkb.go ~/Downloads/meta.json \
            platform_tools/android/apps/skqp/src/main/assets/gmkb

Run as an executable
--------------------

1.  Build the SkQP program, load files on the device, and run skqp:

        ninja -C out/${arch}-rel skqp
        adb shell "cd /data/local/tmp; rm -rf gmkb report"
        adb push platform_tools/android/apps/skqp/src/main/assets/gmkb \
            /data/local/tmp/
        adb push out/${arch}-rel/skqp /data/local/tmp/
        adb shell "cd /data/local/tmp; ./skqp gmkb report"

2.  Produce a one-page error report if there are errors:

        rm -rf /tmp/report
        if adb shell test -d /data/local/tmp/report; then
            adb pull /data/local/tmp/report /tmp/
            tools/skqp/make_report.py /tmp/report
        fi

Run as an APK
-------------

[TODO]
