// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f8525816cb596dde1a3855446792c8e0
REG_FIDDLE(Canvas_drawLine_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF9a67be);
    paint.setStrokeWidth(20);
    canvas->skew(1, 0);
    canvas->drawLine({32, 96}, {32, 160}, paint);
    canvas->skew(-2, 0);
    canvas->drawLine({288, 96}, {288, 160}, paint);
}
}  // END FIDDLE
