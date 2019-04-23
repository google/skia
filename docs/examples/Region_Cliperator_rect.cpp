// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=05791751f00b4c2426093fa143b43bc7
REG_FIDDLE(Region_Cliperator_rect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        SkRegion::Cliperator clipper(region, {0, 0, 5, 3});
        auto r = clipper.rect();
        SkDebugf("%14s rect={%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion region;
    debugster("empty region", region);
    region.setRect({1, 2, 3, 4});
    debugster("after set rect", region);
}
}  // END FIDDLE
