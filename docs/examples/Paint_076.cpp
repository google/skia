// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=211a1b14bfa6c4332082c8eab4fbc5fd
REG_FIDDLE(Paint_076, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= path effect\n", paint.getPathEffect() ? '!' : '=');
   paint.setPathEffect(SkCornerPathEffect::Make(10));
   SkDebugf("nullptr %c= path effect\n", paint.getPathEffect() ? '!' : '=');
}
}  // END FIDDLE
