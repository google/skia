// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3d52763e7c0e20c0b1d484a0afa622d2
REG_FIDDLE(Path_061, 256, 140, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path;
    path.moveTo(20, 80);
    path.rConicTo( 60,   0,  60,  60, 0.707107f);
    path.rConicTo(  0, -60,  60, -60, 0.707107f);
    path.rConicTo(-60,   0, -60, -60, 0.707107f);
    path.rConicTo(  0,  60, -60,  60, 0.707107f);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
