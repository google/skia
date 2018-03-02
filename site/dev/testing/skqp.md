SkQP
====

Development APKs of SkQP are kept in Google storage.  Each file in named
with a abbreviated Git hash that points at the commit in the Skia repository it
was built with.

These are universal APKs that contain native libraries for armeabi-v7a,
arm64-v8a, x86, and x86\_64 architectures. The most recent is listed first:

<!--
#!/bin/sh
BRANCH=origin/skqp/dev
for commit in $(git log $BRANCH -30 -\-format=%H) ; do
    SHORT="$(git log -1 -\-format=%h $commit)"
    URL="https://storage.googleapis.com/skia-skqp/skqp-universal-${SHORT}.apk"
    if [ 200 -ne "$(curl -s -o /dev/null -w "%{http_code}" "$URL")" ] ; then
        continue
    fi
    DATE=$(TZ='' git log -\-date='format-local:%Y-%m-%d %H:%M:%S %Z' -1 $commit -\-format=%cd)
    SUBJ=$(git log -1 $commit -\-format='%<(50,trunc)%s' | sed 's/  *$//')
    printf '  * [`%s`](%s)\n    | `%s` | %s\n' "$SHORT" "$URL" "$DATE" "$SUBJ"
done
-->

  * [`0aa4e74e8c`](https://storage.googleapis.com/skia-skqp/skqp-universal-0aa4e74e8c.apk)
    | `2018-03-01 22:44:44 UTC` | Move the rest of Vulkan driver workarounds into ..
  * [`6ce9d8849b`](https://storage.googleapis.com/skia-skqp/skqp-universal-6ce9d8849b.apk)
    | `2018-03-01 22:24:15 UTC` | Cherry-pick https://skia-review.googlesource.com..
  * [`186ccf5147`](https://storage.googleapis.com/skia-skqp/skqp-universal-186ccf5147.apk)
    | `2018-03-01 19:01:32 UTC` | SkQP: turn on VkMakeCopyPipelineTest
  * [`4e8d3a4bb0`](https://storage.googleapis.com/skia-skqp/skqp-universal-4e8d3a4bb0.apk)
    | `2018-03-01 17:20:13 UTC` | Add unit test the explicit tests create a GrVkCo..
  * [`4f0c60f256`](https://storage.googleapis.com/skia-skqp/skqp-universal-4f0c60f256.apk)
    | `2018-02-28 21:11:06 UTC` | Cherry-pick sequence of Vulkan Fixes.
  * [`120ffdd88c`](https://storage.googleapis.com/skia-skqp/skqp-universal-120ffdd88c.apk)
    | `2018-02-23 19:08:26 UTC` | SkQP:  do test filtering correctly
  * [`5eff3287c9`](https://storage.googleapis.com/skia-skqp/skqp-universal-5eff3287c9.apk)
    | `2018-02-22 14:00:28 UTC` | Remove branch lookup from skqp-dev
  * [`103b402a29`](https://storage.googleapis.com/skia-skqp/skqp-universal-103b402a29.apk)
    | `2018-02-21 20:56:10 UTC` | SkQP: run a single test
  * [`38efb0d355`](https://storage.googleapis.com/skia-skqp/skqp-universal-38efb0d355.apk)
    | `2018-02-20 20:48:45 UTC` | Add SKQP bot to build universal APK to master
  * [`d69db48840`](https://storage.googleapis.com/skia-skqp/skqp-universal-d69db48840.apk)
    | `2018-02-13 21:07:09 UTC` | SkQP: relax five tests
  * [`337919990b`](https://storage.googleapis.com/skia-skqp/skqp-universal-337919990b.apk)
    | `2018-02-13 19:33:12 UTC` | SkQP:  debug option, and fix a bug

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
