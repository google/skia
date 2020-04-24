// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3831fb6006a7e0ad5d140c266c22be78
REG_FIDDLE(Region_Cliperator_const_SkRegion_const_SkIRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    region.setRect({1, 2, 3, 4});
    SkRegion::Cliperator clipper(region, {0, 0, 2, 3});
    auto r = clipper.rect();
    SkDebugf("rect={%d,%d,%d,%d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}  // END FIDDLE
