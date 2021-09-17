# skpbench

skpbench is a benchmarking tool for replaying skp or mksp files on android devices.
it achieves a lower variance in framerate by controlling the clock speed and stopping
all other processes that could cause interference.

## Build

skpbench consists of the skpbench binary which must be built for the phone you intend to run on,
and skpbench.py which runs on the machine the phone is connected to via ADB and is the entry point.

The to build skia for android are at https://skia.org/user/build#android and reproduced here.

Download the Android NDK

```
./bin/sk asset download android_ndk_linux /tmp/ndk
```

After this is set up once, build skpbench for your target cpu (assumed to be arm64 here for a Pixel 3)

```
bin/gn gen out/arm64 --args='ndk="/tmp/ndk" target_cpu="arm64" is_debug=false'
ninja -C out/arm64 skpbench
```

## Benchmark an SKP on a connected device.

First, copy the built skpbench binary and an example skp file to the device.
(or pull a skp corpus using instructions in the section below)

```
adb push out/arm64/skpbench /data/local/tmp
adb push /home/nifong/Downloads/foo.skp /data/local/tmp/skps/
```

Run skpbench.py

```
python tools/skpbench/skpbench.py \
  --adb \
  --config gles \
  /data/local/tmp/skpbench \
  /data/local/tmp/skps/foo.skp
```

`--adb` specifies that it should use adb to the only connected device and run skpbench there.
`--force` is necessary because we don't yet have a configuration to monitor vitals on the Pixel 3.
`--config gles` specifies Open GL ES is the backend GPU config to use.

Additional documentation of arguments is printed by `python tools/skpbench/skpbench.py --help`

Output appears in the following format
```
   accum    median       max       min   stddev  samples  sample_ms  clock  metric  config    bench
  0.1834    0.1832    0.1897    0.1707    1.59%      101         50  cpu    ms      gles      foo.skp
```

`accum` is the time taken to draw all frames, divided by the number of frames.
`metric` specifies that the unit is ms (milliseconds per frame)

## MSKP corpus

A manually collected corpus of MSKPs from around 30 top apps (using skia via HWUI) and of about 20
actions in RenderEngine exists in a google cloud storage folder managed by skia/infra/bots/assets/mskp/upload.py

To download the fileset, first determine the highest current version of the fileset

```
gsutil ls gs://skia-assets/assets/mskp/
```

Download the latest version.

```
gsutil cp gs://skia-assets/assets/mskp/5.zip ~/Downloads
```

Unzip the archive and adb push it to the device.

To upload a new version of the corpus, use the steps above to download and unzip the last version, change the
content however you need, then Use the upload tool, passing the directory of the altered archive (not a zip file).
Note that you must provide it as an absolute path.

```
python upload.py --target_dir=/home/nifong/scratch/new_mskps
```

The upload script should print a version number.
Finally, submit something like https://skia-review.googlesource.com/c/skia/+/304376
to point jobs at the new version.

## Production

skpbench is run as a tryjob from gerrit, where it uploads the results to perf.skia.org.
Once such job name is `Perf-Android-Clang-Pixel4XL-GPU-Adreno640-arm64-Release-All-Android_Skpbench`

Perf results are available by querying with this or similar.
  extra_config = Android_Skpbench
  sub_result = accum_cpu_ms

Example perf query
https://perf.skia.org/e/?queries=extra_config%3DAndroid_Skpbench%26sub_result%3Daccum_cpu_ms
