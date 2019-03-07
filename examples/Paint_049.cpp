// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Paint_049, 256, 256, true, 0);
// HASH=0c4446c0870b5c7b5a2efe77ff92afb8
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(5);
    paint.setStrokeWidth(-1);
    SkDebugf("5 %c= paint.getStrokeWidth()\n", 5 == paint.getStrokeWidth() ? '=' : '!');
}
}
