SkQP
====

Development APKs of SkQP are kept in Google storage.  Each file in named
with a abbreviated Git hash that points at the commit in the Skia repository it
was built with.

These are universal APKs that contain native libraries for armeabi-v7a,
arm64-v8a, x86, and x86\_64 architectures. The most recent is listed first.

The listing can be found here:
[https://storage.googleapis.com/skia-skqp/apklist](https://storage.googleapis.com/skia-skqp/apklist)

To run tests:

    adb install -r skqp-universal-{APK_SHA_HERE}.apk
    adb logcat -c
    adb shell am instrument -w org.skia.skqp

Monitor the output with:

    adb logcat org.skia.skqp skia DEBUG "*:S"

Note the test's output path on the device.  It will look something like this:

    01-23 15:22:12.688 27158 27173 I org.skia.skqp:
    output written to "/storage/emulated/0/Android/data/org.skia.skqp/files/output"

Retrieve and view the report with:

    OUTPUT_LOCATION="/storage/emulated/0/Android/data/org.skia.skqp/files/output"
    adb pull $OUTPUT_LOCATION /tmp/

Open the file `/tmp/output/skqp_report/report.html` .

* * *

For more information about building your own APK, refer to
https://skia.googlesource.com/skia/+/master/tools/skqp/README.md
