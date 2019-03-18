// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e12575ffcd262f2364e0e6bece98a825
REG_FIDDLE(Region_setRect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRegion region({1, 2, 3, 4});
    SkDebugf("region is %s" "empty\n", region.isEmpty() ? "" : "not ");
    bool setEmpty = region.setRect({1, 2, 1, 4});
    SkDebugf("region is %s" "empty\n", region.isEmpty() ? "" : "not ");
    SkDebugf("setEmpty: %s\n", setEmpty ? "true" : "false");
}
}  // END FIDDLE
