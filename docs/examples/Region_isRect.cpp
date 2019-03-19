// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b6adbdddf7fe45a1098121c4e5fd57ea
REG_FIDDLE(Region_isRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, const SkRegion& region) -> void {
                SkDebugf("%s: region is %s" "rect\n", label, region.isRect() ? "" : "not ");
    };
    SkRegion region;
    debugster("initial", region);
    region.setRect({1, 2, 3, 4});
    debugster("set rect", region);
    region.setEmpty();
    debugster("set empty", region);
}
}  // END FIDDLE
