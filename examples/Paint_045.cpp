// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Paint_045, 256, 256, true, 0);
// HASH=1c5e18c3c0102d2dac86a78ba8c8ce01
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("SkPaint::kFill_Style %c= paint.getStyle()\n",
            SkPaint::kFill_Style == paint.getStyle() ? '=' : '!');
}
}
