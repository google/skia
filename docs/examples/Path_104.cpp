// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=df8160dd7ac8aa4b40fce7286fe49952
REG_FIDDLE(Path_104, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(100, 100);
    path.quadTo(100, 20, 20, 100);
    SkMatrix matrix;
    matrix.setRotate(36, 100, 100);
    path.transform(matrix);
    SkPoint last;
    path.getLastPt(&last);
    SkDebugf("last point: %g, %g\n", last.fX, last.fY);
}
}  // END FIDDLE
