// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=257c9473db7a2b3a0fb2b9e2431e59a6
REG_FIDDLE(Paint_074, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("isSrcOver %c= true\n", paint.isSrcOver() ? '=' : '!');
   paint.setBlendMode(SkBlendMode::kSrc);
   SkDebugf("isSrcOver %c= true\n", paint.isSrcOver() ? '=' : '!');
}
}  // END FIDDLE
