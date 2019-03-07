// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=985ff654a6b67288d322c748132a088e
REG_FIDDLE(Region_017, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion* region = new SkRegion({1, 2, 3, 4});
    SkRegion region2(*region);
    delete region;
    auto r = region2.getBounds();
    SkDebugf("region2 bounds: {%d,%d,%d,%d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}  // END FIDDLE
