// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9a39c56e95b19a657133b7ad1fe0cf03
REG_FIDDLE(Path_042, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
            const SkRect& bounds = path.computeTightBounds();
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
