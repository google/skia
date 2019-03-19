// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=304148c50c91490bfd58e9222342419c
REG_FIDDLE(Bitmap_extractSubset, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkIRect bounds, s;
    source.getBounds(&bounds);
    SkDebugf("bounds: %d, %d, %d, %d\n", bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    SkBitmap subset;
    for (int left: { -100, 0, 100, 1000 } ) {
         for (int right: { 0, 100, 1000 } ) {
             SkIRect b = SkIRect::MakeLTRB(left, 100, right, 200);
             bool success = source.extractSubset(&subset, b);
             SkDebugf("subset: %4d, %4d, %4d, %4d  ", b.fLeft, b.fTop, b.fRight, b.fBottom);
             SkDebugf("success; %s", success ? "true" : "false");
             if (success) {
                 subset.getBounds(&s);
                 SkDebugf("  subset: %d, %d, %d, %d", s.fLeft, s.fTop, s.fRight, s.fBottom);
             }
             SkDebugf("\n");
         }
    }
}
}  // END FIDDLE
