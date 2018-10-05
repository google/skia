#!/bin/bash
# Copyright 2018 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Assumes this is in a docker container with a skia repo mounted at /SRC and a folder containing the
# built APK to be in /OUT.
# Additionally, this assumes that the docker container was run to have an android emulator running
# that is accesible with adb.
# This script in particular doesn't care about arm vs x86, but the current incarnation has an x86
# emulator and so the apk should be built for that.
#
# Example usage:
#

set -ex

# Wait for boot
timeout 60 adb wait-for-device shell 'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done'
# Some extra sleep to make sure the emulator is awake and ready for installation
sleep 10

adb install -r /OUT/*.apk
adb logcat -c
adb shell am instrument -w org.skia.skqp
adb logcat -d TestRunner org.skia.skqp skia DEBUG "*:S"
