// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d610437a65dd3e952719efe605cbd0c7
REG_FIDDLE(Rect_intersect_3, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkRect result;
    bool intersected = result.intersect({ 10, 40, 50, 80 }, { 30, 60, 70, 90 });
    SkDebugf("%s intersection: %g, %g, %g, %g\n", intersected ? "" : "no ",
             result.left(), result.top(), result.right(), result.bottom());
}
}  // END FIDDLE
