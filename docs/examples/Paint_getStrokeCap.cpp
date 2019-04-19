// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=aabf9baee8e026fae36fca30e955512b
REG_FIDDLE(Paint_getStrokeCap, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkDebugf("kButt_Cap %c= default stroke cap\n",
            SkPaint::kButt_Cap == paint.getStrokeCap() ? '=' : '!');
}
}  // END FIDDLE
