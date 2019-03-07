// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Paint_048, 256, 256, true, 0);
// HASH=99aa73f64df8bbf06e656cd891a81b9e
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("0 %c= paint.getStrokeWidth()\n", 0 == paint.getStrokeWidth() ? '=' : '!');
}
}
