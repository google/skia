#!/bin/bash

set -e -x

adb root
adb shell cd /data/local/tmp/; ./perfetto.sh
adb pull /data/misc/perfetto-traces/nanobench.perfetto-trace