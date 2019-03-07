// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8d5c88478528584913867ada423e0d59
REG_FIDDLE(RRect_027, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    for (SkScalar radiusX : { SK_ScalarNaN, SK_ScalarInfinity, 100.f, 50.f, 25.f} ) {
        SkRRect rrect1 = SkRRect::MakeRectXY({10, 20, 60, 220}, radiusX, 200);
        SkDebugf("left corner: (%g) %g\n", radiusX, rrect1.radii(SkRRect::kUpperLeft_Corner).fX);
    }
}
}  // END FIDDLE
