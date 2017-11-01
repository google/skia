
SkQP
====

**Motivation**: Test an Android device’s GPU and OpenGLES & Vulkan drivers with
Skia and Skia’s existing unit & rendering tests.

How To Use SkQP on your Android device:

(These are my current notes for the current testing workflow.  I'll clean this
up later, after I package everything up in an APK.)

1.  Download meta.json from [https://goo.gl/jBw3Dd](https://goo.gl/jBw3Dd)

2.  Checkout Skia, then go to the source directory:

        cd $SKIA_SOURCE_DIRECTORY

3.  Generate the GM KnowledgeBase in /tmp/gmkb:

        rm -rf /tmp/gmkb
        go run tools/skqp/make_gmkb.go ~/Downloads/meta.json /tmp/gmkb

4.  Build the SkQP program for your device:

        mkdir -p out/arm64
        cat > out/arm64/args.gn << EOF
            ndk = "${HOME}/ndk"
            ndk_api = 24
            target_cpu = "arm64"
            extra_cflags = [ "-DGR_GL_CHECK_ERROR=0" ]
            is_debug = false
        EOF
        tools/git-sync-deps
        bin/gn gen out/arm64
        ninja -C out/arm64 skqp

5.  Load resources to the device, run gmkb, and pull down the results:

        adb shell "cd /data/local/tmp; rm -rf resources gmkb report skqp"
        adb push out/arm64/skqp resources /tmp/gmkb /data/local/tmp/
        adb shell "cd /data/local/tmp; GMK_DIR=gmkb GMK_REPORT=report ./skqp"
        rm -rf /tmp/report
        adb pull /data/local/tmp/report /tmp/

6.  Produce a one-page error report (if there is anything in /tmp/report)

        {
          if [ -d /tmp/report ]; then
            echo '<style>img{max-width:48%;border:1px green solid;}</style>'
            for x in $(cd /tmp/report && echo */*); do
              printf '\n<h2>%s</h2>\n' $x
              sed 's|="../../|="|g' < /tmp/report/$x/report.html
              printf '<a href="%s">%s</a>\n' /tmp/gmkb/$(basename $x)/min.png min
              printf '<a href="%s">%s</a>\n' /tmp/gmkb/$(basename $x)/max.png max
            done
          fi
        } > /tmp/report/report.html
