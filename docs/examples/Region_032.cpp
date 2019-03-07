// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fc793a14ed76c096a68a755c963c1ee0
REG_FIDDLE(Region_032, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rects[] = { {10, 10, 40, 40}, {20, 20, 50, 50}, {30, 30, 60, 60} };
    SkRegion region;
    region.setRects(rects, SK_ARRAY_COUNT(rects));
    canvas->drawRegion(region, SkPaint());
    region.setEmpty();
    for (auto add : rects) {
        region.op(add, SkRegion::kUnion_Op);
    }
    region.translate(100, 0);
    canvas->drawRegion(region, SkPaint());
}
}  // END FIDDLE
