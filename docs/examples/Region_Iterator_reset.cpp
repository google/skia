// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d153f87bd518a4ab947b7e407ea1db79
REG_FIDDLE(Region_Iterator_reset, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* label, SkRegion::Iterator& iter) -> void {
        SkDebugf("%14s: done=%s\n", label, iter.done() ? "true" : "false");
    };
    SkRegion region;
    SkRegion::Iterator iter(region);
    debugster("empty region", iter);
    region.setRect({1, 2, 3, 4});
    debugster("after set rect", iter);
    iter.reset(region);
    debugster("after reset", iter);
}
}  // END FIDDLE
