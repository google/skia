// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3bbcc7eec19c808a8167bbcc987199f8
REG_FIDDLE(Region_010, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    SkIRect rects[] = {{1, 2, 3, 4}, {5, 6, 7, 8}};
    region.setRects(rects, SK_ARRAY_COUNT(rects));
    SkRegion::Cliperator clipper(region, {0, 3, 8, 7});
    do {
        auto r2 = clipper.rect();
        SkDebugf("rect={%d,%d,%d,%d}\n", r2.fLeft, r2.fTop, r2.fRight, r2.fBottom);
        clipper.next();
    } while (!clipper.done());
}
}  // END FIDDLE
