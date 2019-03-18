// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=60ee3eb747474f5781b0f0dd3a17a866
REG_FIDDLE(Path_quadTo, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path;
    path.moveTo(0, -10);
    for (int i = 0; i < 128; i += 16) {
        path.quadTo( 10 + i, -10 - i,  10 + i,       0);
        path.quadTo( 14 + i,  14 + i,       0,  14 + i);
        path.quadTo(-18 - i,  18 + i, -18 - i,       0);
        path.quadTo(-22 - i, -22 - i,       0, -22 - i);
    }
    path.offset(128, 128);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
