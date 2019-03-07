// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=4549dcda3e0f9a41b3daee0ed37deca8
REG_FIDDLE(Region_014, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    SkIRect r = region.getBounds();
    SkDebugf("region bounds: {%d, %d, %d, %d}\n", r.fLeft, r.fTop, r.fRight, r.fBottom);
}
}  // END FIDDLE
