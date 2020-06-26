# Copyright 2020 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#!/bin/bash
mkdir -p out

# download perfetto trace_processor
if [ ! -f "trace_processor" ]; then
    wget https://get.perfetto.dev/trace_processor
    chmod +x trace_processor
fi

adb root
# get out of the lock screen
adb shell input keyevent MENU
adb shell input keyevent MENU
adb shell am force-stop org.skia.skottie
renderer_names=(lottie_hw lottie_sw skottie)
# iterate over lottie HW and skottie renderers
for renderer in {0,2}
do
echo "renderer " ${renderer_names[${renderer}]}
# iterate over each of the 15 lottie files
for file in {0..14}
do
echo "show file " ${file}
# start perfetto first (before the app) to give it a chance to capture startup metrics/shader compile and first frame rendering
./collect.sh &
perfetto_pid=$!
# give one second for the perfetto script to start
sleep 1
adb shell am start -n org.skia.skottie/.PerfActivity --ei renderer ${renderer} --ei file ${file}
#wait for perfetto to finish (~10s)
wait $perfetto_pid
adb shell am force-stop org.skia.skottie
./trace_processor --run-metrics=skottie_metric.sql --metrics-output=json trace > out/data_${renderer_names[${renderer}]}_${file}.json
mv trace out/trace_${renderer_names[${renderer}]}_${file}
done
done
echo All done
