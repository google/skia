#!/bin/sh
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Create a 3 second long animation from the Raster output of fiddle at 15 fps.
FPS=15
DURATION=3
FRAMES=$((DURATION * FPS))
mkdir -p /tmp/animation
for i in $(seq -f "%05g" 0 $FRAMES)
do
    ./out/Release/fiddle --duration $DURATION --frame `bc -l <<< "$i/$FRAMES"` | ./tools/fiddle/parse-fiddle-output
    cp /tmp/fiddle_Raster.png /tmp/animation/image-"$i".png
done
cd /tmp/animation; ffmpeg -r $FPS -pattern_type glob -i '*.png' -c:v libvpx-vp9 -lossless 1 output.webm
