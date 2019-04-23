// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a2db43ee3cbf6893e9b23927fb44298a
REG_FIDDLE(Region_Iterator_Iterator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion::Iterator iter;
    SkRegion region;
    region.setRect({1, 2, 3, 4});
    iter.reset(region);
    auto r = iter.rect();
    SkDebugf("rect={%d,%d,%d,%d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}  // END FIDDLE
