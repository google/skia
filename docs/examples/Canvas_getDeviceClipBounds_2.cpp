// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
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
