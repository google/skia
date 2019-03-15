// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0e7c58ab5d3bcfb36b1f8464cf6c7d89
REG_FIDDLE(Region_006, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    SkRegion::Iterator iter(region);
    auto r1 = iter.rect();
    SkDebugf("rect={%d,%d,%d,%d}\n", r1.fLeft, r1.fTop, r1.fRight, r1.fBottom);
    region.setRect({1, 2, 3, 4});
    iter.rewind();
    auto r2 = iter.rect();
    SkDebugf("rect={%d,%d,%d,%d}\n", r2.fLeft, r2.fTop, r2.fRight, r2.fBottom);
}
}  // END FIDDLE
