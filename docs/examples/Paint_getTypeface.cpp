#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5ce718e5a184baaac80e7098d7dad67b
REG_FIDDLE(Paint_getTypeface, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= typeface\n", paint.getTypeface() ? '!' : '=');
   paint.setTypeface(SkTypeface::MakeFromName("monospace", SkFontStyle()));
   SkDebugf("nullptr %c= typeface\n", paint.getTypeface() ? '!' : '=');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
