// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b30d4780475d113a7fed3637af7f0db1
REG_FIDDLE(IPoint_minus_operator, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint test[] = { {0, -0}, {-1, -2},
                       { SK_MaxS32, SK_MinS32 },
                       { SK_NaN32, SK_NaN32 } };
    for (const SkIPoint& pt : test) {
        SkIPoint negPt = -pt;
        SkDebugf("pt: %d, %d  negate: %d, %d\n", pt.fX, pt.fY, negPt.fX, negPt.fY);
    }
}
}  // END FIDDLE
