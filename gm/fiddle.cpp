/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

SkPath convertRoundRectToPath(float centerX, float centerY, float width, float height, float radius) {
    SkPath path;
    float hw = width * 0.5;
    float hh = height * 0.5;
    float x = centerX - hw;
    float y = centerY - hh;
    if (radius > hw) {
      radius = hw;
    }
    if (radius > hh) {
      radius = hh;
    }

  float percent = 0.552284777;
  //    A-----B
  //  H         C
  //  G         D
  //    F-----E
  // begins at the C point
  float right = x + width;
  float bottom = y + height;
  float xlw = x + radius;
  float xrw = right - radius;
    float ytw = y + radius;
    float ybw = bottom - radius;
    float corner = radius * (1 - percent);
  path.moveTo(right, ytw);
  path.lineTo(right, ybw);
  path.cubicTo(right, bottom - corner, right - corner, bottom, xrw, bottom);
  path.lineTo(xlw, bottom);
  path.cubicTo(x + corner, bottom, x, bottom - corner, x, ybw);
  path.lineTo(x, ytw);
  path.cubicTo(x, y + corner, x + corner, y, xlw, y);
  path.lineTo(xrw, y);
  path.cubicTo(right - corner, y, right, y + corner, right, ytw);
  path.close();
  return path;
}


// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    //paint.setColor(0, 0, 0, 1.0);
    paint.setStyle(SkPaint::kFill_Style);

    //paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(4.0);
    SkPath path = convertRoundRectToPath(100, 100, 90, 50, 80);

    canvas->drawPath(path, paint);
}
