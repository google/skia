SkQP
====

Development APKs of SkQP are kept in Google storage.  Each file in named
with a abbreviated Git hash that points at the commit in the Skia repository it
was built with.

These are universal APKs that contain native libraries for armeabi-v7a,
arm64-v8a, x86, and x86\_64 architectures. The most recent is listed first.

The listing can be found here:
[https://storage.googleapis.com/skia-skqp/apklist](https://storage.googleapis.com/skia-skqp/apklist)

If you are looking at Android CTS failures, use the most recent commit on the
`origin/skqp/release` branch.

To run tests:

    adb install -r skqp-universal-{APK_SHA_HERE}.apk
    adb logcat -c
    adb shell am instrument -w org.skia.skqp

Monitor the output with:

    adb logcat TestRunner org.skia.skqp skia DEBUG "*:S"

Note the test's output path on the device.  It will look something like this:

    01-23 15:22:12.688 27158 27173 I org.skia.skqp:
    output written to "/storage/emulated/0/Android/data/org.skia.skqp/files/skqp_report_2019-02-28T102058"

Retrieve and view the report with:

    OUTPUT_LOCATION="/storage/emulated/0/Android/data/org.skia.skqp/files/skqp_report_2019-02-28T102058"
    adb pull "$OUTPUT_LOCATION" /tmp/

(Your value of `$OUTPUT_LOCATION` will differ from mine.

Open the file `/tmp/output/skqp_report_2019-02-28T102058/report.html` .

**Zip up that directory to attach to a bug report:**

    cd /tmp
    zip -r skqp_report_2019-02-28T102058.zip skqp_report_2019-02-28T102058
    ls -l skqp_report_2019-02-28T102058.zip

* * *

For more information about building your own APK, refer to
https://skia.googlesource.com/skia/+/master/tools/skqp/README.md
