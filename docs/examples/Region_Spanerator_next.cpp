// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=03d02180fee5f64ec4a3347e118fb2ec
REG_FIDDLE(Region_Spanerator_next, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        SkRegion::Spanerator spanner(region, 3, 2, 4);
        int left, right;
        bool result = spanner.next(&left, &right);
        SkDebugf("%14s: result=%s", label, result ? "true" : "false");
        if (result) SkDebugf(" left=%d right=%d", left, right);
        SkDebugf("\n");
    };
    SkRegion region;
    debugster("empty region", region);
    region.setRect({1, 2, 3, 4});
    debugster("after set rect", region);
}
}  // END FIDDLE
