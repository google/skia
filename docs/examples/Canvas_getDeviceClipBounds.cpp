// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=556832ac5711af662a98c21c547185e9
REG_FIDDLE(Canvas_getDeviceClipBounds, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkCanvas device(256, 256);
    canvas = &device;
    SkIRect bounds = canvas->getDeviceClipBounds();
    SkDebugf("left:%d  top:%d  right:%d  bottom:%d\n",
            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    SkPoint clipPoints[]  = {{30, 130}, {120, 130}, {120, 230} };
    SkPath clipPath;
    clipPath.addPoly(clipPoints, SK_ARRAY_COUNT(clipPoints), true);
    canvas->save();
    canvas->clipPath(clipPath);
    bounds = canvas->getDeviceClipBounds();
    SkDebugf("left:%d  top:%d  right:%d  bottom:%d\n",
            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    canvas->restore();
    canvas->scale(1.f/2, 1.f/2);
    canvas->clipPath(clipPath);
    bounds = canvas->getDeviceClipBounds();
    SkDebugf("left:%d  top:%d  right:%d  bottom:%d\n",
            bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
}
}  // END FIDDLE
