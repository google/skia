// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d38aaf12c6ff5b8d901a2201bcee5476
REG_FIDDLE(Path_cubicTo_2, 256, 84, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPoint pts[] = { {20, 20}, {300, 80}, {-140, 90}, {220, 10} };
    SkPath path;
    path.moveTo(pts[0]);
    path.cubicTo(pts[1], pts[2], pts[3]);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
