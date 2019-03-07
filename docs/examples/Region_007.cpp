// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bbc3c454a21186e2a16e843a5b061c44
REG_FIDDLE(Region_007, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    SkIRect rects[] = {{1, 2, 3, 4}, {3, 4, 5, 6}};
    region.setRects(rects, SK_ARRAY_COUNT(rects));
    SkRegion::Iterator iter(region);
    auto r = iter.rect();
    SkDebugf("rect={%d,%d,%d,%d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
    auto b = iter.rgn()->getBounds();
    SkDebugf("bounds={%d,%d,%d,%d}\n", b.fLeft, b.fTop, b.fRight, b.fBottom);
}
}  // END FIDDLE
