// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=10ef0de39e8553dd97cf8668ce185070
REG_FIDDLE(Region_023, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        SkDebugf("%14s: region is %s" "empty\n", label, region.isEmpty() ? "" : "not ");
    };
    SkRegion region;
    debugster("initial", region);
    region.setRect({1, 2, 3, 4});
    debugster("set rect", region);
    region.setEmpty();
    debugster("set empty", region);
}
}  // END FIDDLE
