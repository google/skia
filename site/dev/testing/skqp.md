SkQP
====

The most recent development APK is
[skqp-universal-337919990b.apk](https://storage.googleapis.com/skia-skqp/skqp-universal-337919990b.apk).

To run tests:

    adb install -r skqp-universal-337919990b.apk
    adb logcat -c
    adb shell am instrument -w org.skia.skqp/android.support.test.runner.AndroidJUnitRunner

Monitor the output with:

    adb logcat org.skia.skqp skia "*:S"

Note the test's output path on the device.  It will look something like this:

    01-23 15:22:12.688 27158 27173 I org.skia.skqp:
    output written to "/storage/emulated/0/Android/data/org.skia.skqp/files/output"

Retrieve and view the report with:

    OUTPUT_LOCATION="/storage/emulated/0/Android/data/org.skia.skqp/files/output"
    adb pull $OUTPUT_LOCATION /tmp/
    tools/skqp/sysopen.py /tmp/output/skqp_report/report.html

* * *

For more information about building your own APK, refer to
https://skia.googlesource.com/skia/+/master/tools/skqp/README.md
