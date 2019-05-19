#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bb179ec5698ec1398ff18f3657ab73f7
REG_FIDDLE(Paint_setHinting, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint1, paint2;
    paint2.setHinting(SkFontHinting::kNormal);
    SkDebugf("paint1 %c= paint2\n", paint1 == paint2 ? '=' : ':');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
