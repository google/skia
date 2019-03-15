// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3daa83fca809b9ec6560d2ef9e2da5e6
REG_FIDDLE(Region_015, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        auto r = region.getBounds();
        SkDebugf("%14s: {%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region({1, 2, 3, 4});
    SkRegion region2(region);
    debugster("region bounds", region);
    debugster("region2 bounds", region2);
    region.setEmpty();
    SkDebugf("    after region set empty:\n");
    debugster("region bounds", region);
    debugster("region2 bounds", region2);
}
}  // END FIDDLE
