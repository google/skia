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
cipd auth-login
python2 infra/bots/assets/android_ndk_linux/download.py  -t /tmp/ndk

```

After this is set up once, build skpbench for your target cpu (assumed to be arm64 here for a Pixel 3)

```
bin/gn gen out/arm64 --args='ndk="/tmp/ndk" target_cpu="arm64" is_debug=false'
ninja -C out/arm64 skpbench
```

## Benchmark an SKP on a connected device.

First, copy the built skpbench binary and an example skp file to the device.

```
adb push out/arm64/skpbench /data/local/tmp
adb push /home/nifong/Downloads/foo.skp /data/local/tmp/skps/
```

Run skpbench.py (in my case on a Pixel 3)

```
python tools/skpbench/skpbench.py \
  --adb \
  --force \
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

## Production

skpbench is run as a tryjob from gerrit, where it uploads the results to perf.skia.org.
TODO(nifong, csmartdalton): elaborate on this section.