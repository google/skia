// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=358d9b6060b528b0923c007420f09c13
REG_FIDDLE(Path_059, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPoint conicPts[] = {{20, 150}, {120, 10}, {220, 150}};
    canvas->drawLine(conicPts[0], conicPts[1], paint);
    canvas->drawLine(conicPts[1], conicPts[2], paint);
    SkColor colors[] = { 0xff88ff00, 0xff0088bb, 0xff6600cc, 0xffbb3377 };
    paint.setStrokeWidth(3);
    SkScalar weight = 0.5f;
    for (unsigned i = 0; i < SK_ARRAY_COUNT(colors); ++i) {
        SkPath path;
        path.moveTo(conicPts[0]);
        path.conicTo(conicPts[1], conicPts[2], weight);
        paint.setColor(colors[i]);
        canvas->drawPath(path, paint);
        weight += 0.25f;
   }
}
}  // END FIDDLE
