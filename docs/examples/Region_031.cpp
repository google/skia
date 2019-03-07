// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5b31a1b077818a8150ad50f3b19e7bfe
REG_FIDDLE(Region_031, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, bool success, SkRegion& region) -> void {
        auto r = region.getBounds();
        SkDebugf("%14s: success:%s {%d,%d,%d,%d}\n", label, success ? "true" : "false",
                 r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region;
    bool success = region.setRect(1, 2, 3, 4);
    debugster("set to: 1,2,3,4", success, region);
    success = region.setRect(3, 2, 1, 4);
    debugster("set to: 3,2,1,4", success, region);
}
}  // END FIDDLE
