#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b56b70c7ea2453c41bfa58b626953bed
REG_FIDDLE(Paint_010, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("SkFontHinting::kNormal %c= paint.getHinting()\n",
            SkFontHinting::kNormal == paint.getHinting() ? '=' : ':');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
