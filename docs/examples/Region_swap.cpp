// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ae67b7b4c198b46c58e48f5af061c8f1
REG_FIDDLE(Region_swap, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        auto r = region.getBounds();
        SkDebugf("%14s: {%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region1({1, 2, 3, 4});
    SkRegion region2;
    region1.swap(region2);
    debugster("region1 bounds", region1);
    debugster("region2 bounds", region2);
}
}  // END FIDDLE
