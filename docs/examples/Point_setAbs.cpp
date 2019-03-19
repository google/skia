// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7f70860e820b67a347cff03c00488426
REG_FIDDLE(Point_setAbs, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPoint test[] = { {0.f, -0.f}, {-1, -2},
                       { SK_ScalarInfinity, SK_ScalarNegativeInfinity },
                       { SK_ScalarNaN, -SK_ScalarNaN } };
    for (const SkPoint& pt : test) {
        SkPoint absPt;
        absPt.setAbs(pt);
        SkDebugf("pt: %g, %g  abs: %g, %g\n", pt.fX, pt.fY, absPt.fX, absPt.fY);
    }
}
}  // END FIDDLE
