// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=71ac24b7d91ac5ca7c14b43930d5f85d
REG_FIDDLE(Region_quickReject, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region({1, 2, 3, 4});
    SkIRect test = {4, 2, 5, 3};
    SkDebugf("quickReject 1: %s\n", region.quickReject(test) ? "true" : "false");
    region.op({1, 4, 3, 6}, SkRegion::kUnion_Op);
    SkDebugf("quickReject 2: %s\n", region.quickReject(test) ? "true" : "false");
    region.op({4, 7, 5, 8}, SkRegion::kUnion_Op);
    SkDebugf("quickReject 3: %s\n", region.quickReject(test) ? "true" : "false");
}
}  // END FIDDLE
