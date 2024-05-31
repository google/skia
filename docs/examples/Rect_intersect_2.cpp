// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Rect_intersect_2, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect leftRect =  { 10, 40, 50, 80 };
    SkDebugf("%s intersection: ", leftRect.intersect({30, 60, 70, 90}) ? "" : "no ");
    SkDebugf("%g, %g, %g, %g\n", leftRect.left(), leftRect.top(),
                                 leftRect.right(), leftRect.bottom());
}
}  // END FIDDLE
