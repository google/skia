// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=31bf751d0a8ddf176b871810820d8199
REG_FIDDLE(Paint_getStrokeJoin, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("kMiter_Join %c= default stroke join\n",
            SkPaint::kMiter_Join == paint.getStrokeJoin() ? '=' : '!');
}
}  // END FIDDLE
