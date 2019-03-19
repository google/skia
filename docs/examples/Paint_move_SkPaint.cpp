// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8ed1488a503cd5282b86a51614aa90b1
REG_FIDDLE(Paint_move_SkPaint, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    float intervals[] = { 5, 5 };
    paint.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 2.5f));
    SkPaint dashed(std::move(paint));
    SkDebugf("path effect unique: %s\n", dashed.getPathEffect()->unique() ? "true" : "false");
}
}  // END FIDDLE
