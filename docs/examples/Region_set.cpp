// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b3538117c7ae2cb7de3b42ca45fe1b13
REG_FIDDLE(Region_set, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        auto r = region.getBounds();
        SkDebugf("%14s: {%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region1({1, 2, 3, 4});
    SkRegion region2;
    region2.set(region1);
    debugster("region1 bounds", region1);
    debugster("region2 bounds", region2);
}
}  // END FIDDLE
