// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=466445ed991d86de08587066392d654a
REG_FIDDLE(Path_062, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPoint cubicPts[] = {{20, 150}, {90, 10}, {160, 150}, {230, 10}};
    SkColor colors[] = { 0xff88ff00, 0xff0088bb, 0xff6600cc, 0xffbb3377 };
    for (unsigned i = 0; i < SK_ARRAY_COUNT(colors); ++i) {
        paint.setColor(0x7fffffff & colors[i]);
        paint.setStrokeWidth(1);
        for (unsigned j = 0; j < 3; ++j) {
            canvas->drawLine(cubicPts[j], cubicPts[j + 1], paint);
        }
        SkPath path;
        path.moveTo(cubicPts[0]);
        path.cubicTo(cubicPts[1], cubicPts[2], cubicPts[3]);
        paint.setStrokeWidth(3);
        paint.setColor(colors[i]);
        canvas->drawPath(path, paint);
        cubicPts[1].fY += 30;
        cubicPts[2].fX += 30;
   }
}
}  // END FIDDLE
