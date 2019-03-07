// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=32d51e959d6cc720a74ec4822511e2cd
REG_FIDDLE(Region_002, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion::Iterator& iter, bool addRewind) -> void {
        if (addRewind) {
            bool success = iter.rewind();
            SkDebugf("%14s rewind success=%s\n", label, success ? "true" : "false");
        }
        auto r = iter.rect();
        SkDebugf("%14s rect={%d,%d,%d,%d}\n", label, r.fLeft, r.fTop, r.fRight, r.fBottom);
    };
    SkRegion::Iterator iter;
    debugster("empty iter", iter, true);
    SkRegion region;
    iter.reset(region);
    debugster("empty region", iter, true);
    region.setRect({1, 2, 3, 4});
    iter.reset(region);
    debugster("after set rect", iter, false);
    debugster("after rewind", iter, true);
}
}  // END FIDDLE
