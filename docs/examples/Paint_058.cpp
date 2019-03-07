// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Paint_058, 256, 256, true, 0) {
// HASH=31bf751d0a8ddf176b871810820d8199
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("kMiter_Join %c= default stroke join\n",
            SkPaint::kMiter_Join == paint.getStrokeJoin() ? '=' : '!');
}
}
