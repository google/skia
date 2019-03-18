// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5253910233f7961c30b4c18ab911e917
REG_FIDDLE(Region_copy_const_SkIRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region({1, 2, 3, 4});
    SkRegion region2;
    region2.setRect({1, 2, 3, 4});
    SkDebugf("region %c= region2\n", region == region2 ? '=' : '!');
}
}  // END FIDDLE
