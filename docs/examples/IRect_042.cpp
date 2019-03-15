// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=200422990eded2f754ab9893118f2645
REG_FIDDLE(IRect_042, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIRect leftRect =  { 10, 40, 50, 80 };
    SkDebugf("%s intersection: ", leftRect.intersect(30, 60, 70, 90) ? "" : "no ");
    SkDebugf("%d, %d, %d, %d\n", leftRect.left(), leftRect.top(),
                                 leftRect.right(), leftRect.bottom());
}
}  // END FIDDLE
