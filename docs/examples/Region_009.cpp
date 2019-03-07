// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6cca7b96836266800d852664a1366453
REG_FIDDLE(Region_009, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion& region) -> void {
        SkRegion::Cliperator clipper(region, {0, 0, 5, 5});
        SkDebugf("%14s done=%s\n", label, clipper.done() ? "true" : "false");
    };
    SkRegion region;
    debugster("empty region", region);
    region.setRect({1, 2, 3, 4});
    debugster("after add rect", region);
}
}  // END FIDDLE
