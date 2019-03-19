// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6abb99f849a1f0e33e1dedc00d1c4f7a
REG_FIDDLE(Canvas_getDeviceClipBounds_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect bounds;
    SkDebugf("device bounds empty = %s\n", canvas->getDeviceClipBounds(&bounds)
             ? "false" : "true");
    SkPath path;
    canvas->clipPath(path);
    SkDebugf("device bounds empty = %s\n", canvas->getDeviceClipBounds(&bounds)
             ? "false" : "true");
}
}  // END FIDDLE
