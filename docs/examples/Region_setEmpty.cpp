// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1314f7250963775c5ee89cc5981eee24
REG_FIDDLE(Region_setEmpty, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        auto r = region.getBounds();
        SkDebugf("%14s: {%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region({1, 2, 3, 4});
    debugster("region bounds", region);
    region.setEmpty();
    SkDebugf("    after region set empty:\n");
    debugster("region bounds", region);
}
}  // END FIDDLE
