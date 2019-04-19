// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b2db0573aacf99ca52776c5522459d02
REG_FIDDLE(IRect_intersect_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect result;
    bool intersected = result.intersect({ 10, 40, 50, 80 }, { 30, 60, 70, 90 });
    SkDebugf("%s intersection: %d, %d, %d, %d\n", intersected ? "" : "no ",
             result.left(), result.top(), result.right(), result.bottom());
}
}  // END FIDDLE
