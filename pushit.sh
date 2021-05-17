#!/bin/bash

adb push config.pbtx /data/local/tmp/config.pbtx
adb push perfetto.sh /data/local/tmp/perfetto.sh
adb shell chmod +x /data/local/tmp/perfetto.sh