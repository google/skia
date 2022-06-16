// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(pong2, 512, 256, false, 0, 10) {
static SkScalar PingPong(double t, SkScalar period, SkScalar phase,
                         SkScalar ends, SkScalar mid) {
  double value = ::fmod(t + phase, period);
  double half = period / 2.0;
  double diff = ::fabs(value - half);
  return SkDoubleToScalar(ends + (1.0 - diff / half) * (mid - ends));
}

void draw(SkCanvas* canvas) {
  float bX = PingPong(frame * duration, 2.5f, 0.0f, 0, 1) * 472 + 20;
  float bY = PingPong(frame * duration, 2.0f, 0.4f, 0, 1) * 200 + 28;

  SkPaint p;
  p.setColor(SK_ColorWHITE);
  p.setAntiAlias(true);

  canvas->clear(SK_ColorBLACK);
  canvas->drawRect(SkRect::MakeXYWH(492, bY - 15, 10, 30), p);
  canvas->drawRect(SkRect::MakeXYWH(10,  bY - 15, 10, 30), p);
  canvas->drawCircle(bX, bY, 5, p);

  const float intervals[] = { 12, 6 };
  p.setStrokeWidth(5);
  p.setPathEffect(SkDashPathEffect::Make(intervals, std::size(intervals), 0));
  canvas->drawLine({256,0}, {256, 256}, p);
}
}  // END FIDDLE
