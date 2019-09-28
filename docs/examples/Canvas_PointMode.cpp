// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=292b4b2008961b6f612434d3121fc4ce
REG_FIDDLE(Canvas_PointMode, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
  SkPaint paint;
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(10);
  SkPoint points[] = {{64, 32}, {96, 96}, {32, 96}};
  canvas->drawPoints(SkCanvas::kPoints_PointMode, 3, points, paint);
  canvas->translate(128, 0);
  canvas->drawPoints(SkCanvas::kLines_PointMode, 3, points, paint);
  canvas->translate(0, 128);
  canvas->drawPoints(SkCanvas::kPolygon_PointMode, 3, points, paint);
  SkPath path;
  path.addPoly(points, 3, false);
  canvas->translate(-128, 0);
  canvas->drawPath(path, paint);
}
}  // END FIDDLE
