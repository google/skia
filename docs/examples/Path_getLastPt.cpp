// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_getLastPt, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path = SkPathBuilder()
                  .moveTo(100, 100)
                  .quadTo(100, 20, 20, 100)
                  .detach();
    SkMatrix matrix;
    matrix.setRotate(36, 100, 100);
    path = path.makeTransform(matrix);
    SkPoint last = path.getLastPt().value_or(SkPoint{0, 0});
    SkDebugf("last point: %g, %g\n", last.fX, last.fY);
}
}  // END FIDDLE
