// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4082f66a42df11bb20462b232b156bb6
REG_FIDDLE(Quad_b, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPoint quadPts[] = {{20, 150}, {120, 10}, {220, 150}};
    SkColor colors[] = { 0xff88ff00, 0xff0088bb, 0xff6600cc, 0xffbb3377 };
    for (unsigned i = 0; i < SK_ARRAY_COUNT(colors); ++i) {
        paint.setColor(0x7fffffff & colors[i]);
        paint.setStrokeWidth(1);
        canvas->drawLine(quadPts[0], quadPts[1], paint);
        canvas->drawLine(quadPts[1], quadPts[2], paint);
        SkPath path;
        path.moveTo(quadPts[0]);
        path.quadTo(quadPts[1], quadPts[2]);
        paint.setStrokeWidth(3);
        paint.setColor(colors[i]);
        canvas->drawPath(path, paint);
        quadPts[1].fY += 30;
   }
}
}  // END FIDDLE
