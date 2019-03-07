#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6bd705a6e0c5f8ee24f302fe531bfabc
REG_FIDDLE(Paint_103, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextScaleX(1.f / 0.f);
    SkDebugf("text scale %s-finite\n", SkScalarIsFinite(paint.getTextScaleX()) ? "is" : "not");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
