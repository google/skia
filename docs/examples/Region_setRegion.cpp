// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5d75d22bd155576838155762ab040751
REG_FIDDLE(Region_setRegion, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        auto r = region.getBounds();
        SkDebugf("%14s: {%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region({1, 2, 3, 4});
    SkRegion region2;
    region2.setRegion(region);
    debugster("region bounds", region);
    debugster("region2 bounds", region2);
    region2.setEmpty();
    SkDebugf("    after region set empty:\n");
    debugster("region bounds", region);
    debugster("region2 bounds", region2);
}
}  // END FIDDLE
