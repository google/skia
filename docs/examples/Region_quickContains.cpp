// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d8e5eac373e2e7cfc1b8cd0229647ba6
REG_FIDDLE(Region_quickContains, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region({1, 2, 3, 4});
    SkIRect test = {2, 2, 3, 3};
    SkDebugf("quickContains 1: %s\n", region.quickContains(test) ? "true" : "false");
    region.op({1, 4, 3, 6}, SkRegion::kUnion_Op);
    SkDebugf("quickContains 2: %s\n", region.quickContains(test) ? "true" : "false");
    region.op({1, 7, 3, 8}, SkRegion::kUnion_Op);
    SkDebugf("quickContains 3: %s\n", region.quickContains(test) ? "true" : "false");
}
}  // END FIDDLE
