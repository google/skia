// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1a0a839061c69d870acca2bcfbdf1a41
REG_FIDDLE(Canvas_drawPoint_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(0x80349a45);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(100);
  paint.setStrokeCap(SkPaint::kSquare_Cap);
  canvas->scale(1, 1.2f);
  canvas->drawPoint({64, 96}, paint);
  canvas->scale(.6f, .8f);
  paint.setColor(SK_ColorWHITE);
  canvas->drawPoint(106, 120, paint);
}
}  // END FIDDLE
