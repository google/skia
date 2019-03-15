#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=54baed3f6bc4b9c31ba664e27767fdc7
REG_FIDDLE(Paint_012, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setFlags((uint32_t) (SkPaint::kAntiAlias_Flag | SkPaint::kDither_Flag));
    SkDebugf("paint.isAntiAlias()\n", paint.isAntiAlias() ? '!' : '=');
    SkDebugf("paint.isDither()\n", paint.isDither() ? '!' : '=');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
