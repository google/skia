// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=dd4e4dd2aaa8039b2430729c6b3af817
REG_FIDDLE(Path_isFinite, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkPath& path) -> void {
        SkDebugf("%s path is %s" "finite\n", prefix, path.isFinite() ? "" : "not ");
    };
    SkPath path;
    debugster("initial", path);
    path.lineTo(SK_ScalarMax, SK_ScalarMax);
    debugster("after line", path);
    SkMatrix matrix;
    matrix.setScale(2, 2);
    path.transform(matrix);
    debugster("after scale", path);
}
}  // END FIDDLE
