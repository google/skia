// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=53da0295972a418cbc9607bbb17feaa8
REG_FIDDLE(Paint_065, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
   SkPaint paint1, paint2;
   paint1.setShader(SkShader::MakeEmptyShader());
   SkDebugf("shader unique: %s\n", paint1.getShader()->unique() ? "true" : "false");
   paint2.setShader(paint1.refShader());
   SkDebugf("shader unique: %s\n", paint1.getShader()->unique() ? "true" : "false");
}
}  // END FIDDLE
