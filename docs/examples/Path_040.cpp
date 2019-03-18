// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=45c0fc3acb74fab99d544b80eadd10ad
REG_FIDDLE(Path_getBounds, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
            const SkRect& bounds = path.getBounds();
            SkDebugf("%s bounds = %g, %g, %g, %g\n", prefix,
                     bounds.fLeft, bounds.fTop, bounds.fRight, bounds.fBottom);
    };
    SkPath path;
    debugster("empty", path);
    path.addCircle(50, 45, 25);
    debugster("circle", path);
    SkMatrix matrix;
    matrix.setRotate(45, 50, 45);
    path.transform(matrix);
    debugster("rotated circle", path);
}
}  // END FIDDLE
