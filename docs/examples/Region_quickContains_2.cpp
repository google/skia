// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=eb6d290887e1a3a0b051b4d7b012f5e1
REG_FIDDLE(Region_quickContains_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        SkDebugf("%s: %s\n", label, region.quickContains(2, 2, 3, 3) ? "true" : "false");
    };
    SkRegion region({1, 2, 3, 4});
    debugster("quickContains 1", region);
    region.op({1, 4, 3, 6}, SkRegion::kUnion_Op);
    debugster("quickContains 2", region);
    region.op({1, 7, 3, 8}, SkRegion::kUnion_Op);
    debugster("quickContains 3", region);
}
}  // END FIDDLE
