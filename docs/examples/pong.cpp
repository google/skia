// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(pong, 256, 256, false, 0, 10) {
static SkScalar PingPong(double t, SkScalar period, SkScalar phase,
                         SkScalar ends, SkScalar mid) {
  double value = ::fmod(t + phase, period);
  double half = period / 2.0;
  double diff = ::fabs(value - half);
  return SkDoubleToScalar(ends + (1.0 - diff / half) * (mid - ends));
}

void draw(SkCanvas* canvas) {
  canvas->clear(SK_ColorBLACK);
  float ballX = PingPong(frame * duration, 2.5f, 0.0f, 0.0f, 1.0f);
  float ballY = PingPong(frame * duration, 2.0f, 0.4f, 0.0f, 1.0f);

  SkPaint p;
  p.setColor(SK_ColorWHITE);
  p.setAntiAlias(true);

  float bX = ballX * 472 + 20;
  float bY = ballY * 200 + 28;

  if (canvas->recordingContext()) {
    canvas->drawRect(SkRect::MakeXYWH(236, bY - 15, 10, 30), p);
    bX -= 256;
  } else {
    canvas->drawRect(SkRect::MakeXYWH(10, bY - 15, 10, 30), p);
  }
  canvas->drawCircle(bX, bY, 5, p);
}
}  // END FIDDLE
