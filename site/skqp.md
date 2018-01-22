SkQP
====

How to run SkQP:

1.  Get Chromium's depot\_tools, the Android NDK, and the Android SDK.

2.  Do the following:

        export ANDROID_HOME=~/android-sdk  # Or wherever you installed the Android SDK.
        export PATH=~/depot_tools:"$PATH"  # Or wherever you installed depot_tools
        android_ndk=~/android-ndk          # Or wherever you installed the Android NDK.

        git clone https://skia.googlesource.com/skia.git
        cd skia
        git checkout origin/skqp
        tools/git-sync-deps
        mkdir -p out/arm-rel
        cat > out/arm-rel/args.gn << EOF
            ndk = "$android_ndk"
            ndk_api = 26
            target_cpu = "arm"
            skia_embed_resources = true
            is_debug = false
            skia_enable_pdf = false
        EOF
        bin/gn gen out/arm-rel
        platform_tools/android/bin/android_build_app -C out/arm-rel skqp
        adb install -r out/arm-rel/skqp.apk
        adb logcat -c
        adb shell am instrument -w org.skia.skqp/android.support.test.runner.AndroidJUnitRunner

3.  Monitor the output with:

        adb logcat org.skia.skqp skia "*:S"

4.  Retrieve the report with `adb pull $OUTPUT_LOCATION /tmp/`.

