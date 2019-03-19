// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=700b284dbc97785c6a9c9636088713ad
REG_FIDDLE(Paint_setStrokeMiter, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeMiter(8);
    paint.setStrokeMiter(-1);
    SkDebugf("default miter limit == %g\n", paint.getStrokeMiter());
}
}  // END FIDDLE
