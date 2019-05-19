// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9baf247cfcd8272c0ddf6ce93f676b37
REG_FIDDLE(Point_minus_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint test[] = { {0.f, -0.f}, {-1, -2},
                       { SK_ScalarInfinity, SK_ScalarNegativeInfinity },
                       { SK_ScalarNaN, -SK_ScalarNaN } };
    for (const SkPoint& pt : test) {
        SkPoint negPt = -pt;
        SkDebugf("pt: %g, %g  negate: %g, %g\n", pt.fX, pt.fY, negPt.fX, negPt.fY);
    }
}
}  // END FIDDLE
