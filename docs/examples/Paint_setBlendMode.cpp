// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_setBlendMode, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("isSrcOver %c= true\n", paint.isSrcOver() ? '=' : '!');
   paint.setBlendMode(SkBlendMode::kSrc);
   SkDebugf("isSrcOver %c= true\n", paint.isSrcOver() ? '=' : '!');
}
}  // END FIDDLE
