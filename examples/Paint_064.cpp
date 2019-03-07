// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Paint_064, 256, 256, true, 0);
// HASH=09f15b9fd88882850da2d235eb86292f
void draw(SkCanvas* canvas) {
   SkPaint paint;
   SkDebugf("nullptr %c= shader\n", paint.getShader() ? '!' : '=');
   paint.setShader(SkShader::MakeEmptyShader());
   SkDebugf("nullptr %c= shader\n", paint.getShader() ? '!' : '=');
}

}
