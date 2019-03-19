// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=771236c2eadfc2fcd02a3e61a0875d39
REG_FIDDLE(Region_Iterator_next, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    SkIRect rects[] = {{1, 2, 3, 4}, {5, 6, 7, 8}};
    region.setRects(rects, SK_ARRAY_COUNT(rects));
    SkRegion::Iterator iter(region);
    do {
        auto r2 = iter.rect();
        SkDebugf("rect={%d,%d,%d,%d}\n", r2.fLeft, r2.fTop, r2.fRight, r2.fBottom);
        iter.next();
    } while (!iter.done());
}
}  // END FIDDLE
