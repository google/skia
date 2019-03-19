// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bba288f5f77fc8e37e89d2ec08e0ac60
REG_FIDDLE(Path_copy_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path1;
    path1.addRect({10, 20, 30, 40});
    SkPath path2 = path1;
    const SkRect& b1 = path1.getBounds();
    SkDebugf("path1 bounds = %g, %g, %g, %g\n", b1.fLeft, b1.fTop, b1.fRight, b1.fBottom);
    const SkRect& b2 = path2.getBounds();
    SkDebugf("path2 bounds = %g, %g, %g, %g\n", b2.fLeft, b2.fTop, b2.fRight, b2.fBottom);
}
}  // END FIDDLE
