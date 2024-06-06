---
title: 'How to capture an SKP file from the Android Framework'
linkTitle: 'How to capture an SKP file from the Android Framework'
---

## Prerequisites

To set up a newly flashed device for capturing, run the following to make it
possible for the recording process to write its file:

```
adb root
adb remount
```

MSKP files may capture any use of a Skia canvas, and there are two uses in
Android instrumented to capture them. HWUI, which will show the contents of a
single application, and RenderEngine which will show interleaved buffers from
multiple applications and transitions such as portrait-landscape.

## Capturing from HWUI

Set the capture_skp property to enable (but not start) HWUI capture capability.
This will only affect applications started after setting the
capture_skp property so you may have to restart the application you wish to
capture from.

```
adb root
adb shell setprop debug.hwui.capture_skp_enabled true
```

Then, each time you want to capture a file:

First, open the application you will be capturing from. Then, trigger capture
with the following script from the root of your Android tree. (See
https://source.android.com/docs/setup/download.)

```
frameworks/base/libs/hwui/tests/scripts/skp-capture.sh PACKAGE_NAME FRAMES
```

`PACKAGE_NAME` is the name of the component or app you want to capture, for
example: **com.google.android.apps.nexuslauncher**.

`FRAMES` is the number of frames to capture. This is optional and defaults to 1.

## Capturing from RenderEngine

Once before capturing, run the following from the Android root.

```
frameworks/native/libs/renderengine/skia/debug/record.sh rootandsetup
```

To record all frames that RenderEngine handles over the span of 2 seconds.

```
frameworks/native/libs/renderengine/skia/debug/record.sh 2000
```

The output file is copied to your current working directory when the device is
finished serializing it. This can take up to 30 seconds.

There is a small chance that the capture script incorrectly detects that the
file is complete too early and copies a truncated file off the device.
It will be unreadable in the debugger. If you suspect this has happened, it's
likely that you can still retrieve the complete file from the device at
`/data/user/re_skiacapture_*.mskp`

## Reading the file

Open the resulting file in the [Skia Debugger]. For single frame SKPs, you could
also use the [Skia Viewer] to view it, or rasterize it with `dm` (see [Skia
Build Instructions] for how to build `dm`):

```
out/Release/dm --src skp --skps FILENAME.skp -w /tmp --config 8888 gpu pdf --verbose
ls -l /tmp/*/skp/FILENAME.skp.*
```

[Skia Build Instructions]: /docs/user/build
[Skia Debugger]: https://debugger.skia.org
[Skia Viewer]: /docs/user/sample/viewer/
