SkQP
====

Development APKs of SkQP are stored in Google storage.  Each file in named
based on the short commit hash of the skia repository with which the APK was
compiled.

These are universal APKs that contain native libraries for armeabi-v7a,
arm64-v8a, x86, and x86\_64 architectures.

<!-- git log - -format='%h  %ci  %<(30,trunc)%s' origin/skqp/dev -->

  * [`d69db48840`](https://storage.googleapis.com/skia-skqp/skqp-universal-d69db48840.apk)
    2018-02-13 21:07:09 +0000  SkQP: relax five tests` -
  * [`337919990b`](https://storage.googleapis.com/skia-skqp/skqp-universal-337919990b.apk)
    2018-02-13 19:33:12 +0000  SkQP:  debug option, and fix..` -

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

Open the file `/tmp/output/skqp_report/report.html` .

* * *

For more information about building your own APK, refer to
https://skia.googlesource.com/skia/+/master/tools/skqp/README.md
