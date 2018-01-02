#!/bin/bash

set -e -x

arch='arm64'  # Also valid: 'arm', 'x68', 'x64'
platform_tools/android/bin/android_build_app -C out/${arch}-rel skqpapp
echo "adb install -r out/${arch}-rel/skqpapp.apk"
echo "adb shell am instrument -w org.skia.skqp/android.support.test.runner.AndroidJUnitRunner"
