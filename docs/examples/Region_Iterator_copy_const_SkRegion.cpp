// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e317ceca48a6a7504219af58f35d2c95
REG_FIDDLE(Region_Iterator_copy_const_SkRegion, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    region.setRect({1, 2, 3, 4});
    SkRegion::Iterator iter(region);
    auto r = iter.rect();
    SkDebugf("rect={%d,%d,%d,%d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}  // END FIDDLE
