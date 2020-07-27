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

declare -a testnames=("star" "movie_loading" "uk" "white_material_wave_loading"
            "check_animation" "confetti" "gears"
            "hand_sanitizer" "heart_preloader" "i_was_scared_after_that_brouhaha"
            "im_thirsty" "true_will" "workout_monkey_stay_healthy"
            "ripple_loading_animation" "signature" "asdasd" "celebration" "check")

adb root
# get out of the lock screen
adb shell input keyevent MENU
adb shell input keyevent MENU
adb shell setprop persist.traced.enable 1
adb shell setenforce 0
adb shell setprop debug.egl.traceGpuCompletion 1
adb shell am force-stop org.skia.skottie
renderer_names=(lottie_hw lottie_sw skottie)
# iterate over lottie HW and skottie renderers
for renderer in {0,2}
do
echo "renderer " ${renderer_names[${renderer}]}
# iterate over each of the 18 lottie files
for file in {0..17}
do
if [[ $1 == "delay" ]]; then
  # start program first and wait 7 seconds for program to load fully
  echo "waiting to show file " ${file} " " ${testnames[$file]}
  adb shell am start -n org.skia.skottie/.PerfActivity --ei renderer ${renderer} --ei file ${file}
  sleep 7
  ./collect.sh
else
  # start perfetto first (before the app) to give it a chance to capture startup metrics/shader compile and first frame rendering
  ./collect.sh &
  perfetto_pid=$!
  # give one second for the perfetto script to start
  sleep 1
  echo "show file " ${file} " " ${testnames[$file]}
  adb shell am start -n org.skia.skottie/.PerfActivity --ei renderer ${renderer} --ei file ${file}
  #wait for perfetto to finish (~10s)
  wait $perfetto_pid
fi
adb shell am force-stop org.skia.skottie
./trace_processor --run-metrics=skottie_metric.sql --metrics-output=json trace > out/data_${renderer_names[${renderer}]}_${file}_${testnames[$file]}.json
mv trace out/trace_${renderer_names[${renderer}]}_${file}_${testnames[$file]}
done
done
echo All done
