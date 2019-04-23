#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a75bbdb8bb866b125c4c1dd5e967d470
REG_FIDDLE(Paint_setTextScaleX, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextScaleX(0.f / 0.f);
    SkDebugf("text scale %s-a-number\n", SkScalarIsNaN(paint.getTextScaleX()) ? "not" : "is");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
