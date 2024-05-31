// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(RRect_rect, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    for (SkScalar left : { SK_ScalarNaN, SK_ScalarInfinity, 100.f, 50.f, 25.f} ) {
        SkRRect rrect1 = SkRRect::MakeRectXY({left, 20, 60, 220}, 50, 200);
        SkDebugf("left bounds: (%g) %g\n", left, rrect1.rect().fLeft);
    }
}
}  // END FIDDLE
