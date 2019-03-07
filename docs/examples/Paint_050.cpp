// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5de2de0f00354e59074a9bb1a42d5a63
REG_FIDDLE(Paint_050, 384, 170, false, 0) {
void draw(SkCanvas* canvas) {
    SkPoint pts[] = {{ 10, 50 }, { 110, 80 }, { 10, 110 }};
    SkVector v[] = { pts[0] - pts[1], pts[2] - pts[1] };
    SkScalar angle1 = SkScalarATan2(v[0].fY, v[0].fX);
    SkScalar angle2 = SkScalarATan2(v[1].fY, v[1].fX);
    const SkScalar strokeWidth = 20;
    SkScalar miterLimit = 1 / SkScalarSin((angle2 - angle1) / 2);
    SkScalar miterLength = strokeWidth * miterLimit;
    SkPath path;
    path.moveTo(pts[0]);
    path.lineTo(pts[1]);
    path.lineTo(pts[2]);
    SkPaint paint;  // set to default kMiter_Join
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeMiter(miterLimit);
    paint.setStrokeWidth(strokeWidth);
    canvas->drawPath(path, paint);
    paint.setStrokeWidth(1);
    canvas->drawLine(pts[1].fX - miterLength / 2, pts[1].fY + 50,
                     pts[1].fX + miterLength / 2, pts[1].fY + 50, paint);
    canvas->translate(200, 0);
    miterLimit *= 0.99f;
    paint.setStrokeMiter(miterLimit);
    paint.setStrokeWidth(strokeWidth);
    canvas->drawPath(path, paint);
    paint.setStrokeWidth(1);
    canvas->drawLine(pts[1].fX - miterLength / 2, pts[1].fY + 50,
                     pts[1].fX + miterLength / 2, pts[1].fY + 50, paint);
}
}  // END FIDDLE
