// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d6dd0b425aa550f21b938a18c2e1a981
REG_FIDDLE(Bitmap_getSubset, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkIRect bounds;
    source.getBounds(&bounds);
    bounds.inset(100, 100);
    SkBitmap subset;
    source.extractSubset(&subset, bounds);
    SkIRect r = source.getSubset();
    SkDebugf("source: %d, %d, %d, %d\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
    r = subset.getSubset();
    SkDebugf("subset: %d, %d, %d, %d\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}  // END FIDDLE
