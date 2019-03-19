// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3073b3f8ea7252871b6156ff674dc385
REG_FIDDLE(Region_Spanerator_const_SkRegion_int_int_int, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region;
    region.setRect({1, 2, 3, 4});
    SkRegion::Spanerator spanner(region, 3, 2, 4);
    int left, right;
    bool result = spanner.next(&left, &right);
    SkDebugf("result=%s left=%d right=%d\n", result ? "true" : "false", left, right);
}
}  // END FIDDLE
