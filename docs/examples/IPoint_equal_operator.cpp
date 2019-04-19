// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=37ffe2817d720f99e6c252332ce70460
REG_FIDDLE(IPoint_equal_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint test[] = { {0, -0}, {-1, -2}, {SK_MaxS32, -1}, {SK_NaN32, -1} };
    for (const SkIPoint& pt : test) {
        SkDebugf("pt: %d, %d  %c= pt\n", pt.fX, pt.fY, pt == pt ? '=' : '!');
    }
}
}  // END FIDDLE
