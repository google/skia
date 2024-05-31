// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_rConicTo, 256, 140, false, 0) {
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
