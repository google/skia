// Copyright 2019 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"

#include <array>

REG_FIDDLE(Cubic, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    auto cubicPts = std::to_array<SkPoint>(
            {SkPoint{20, 150}, SkPoint{90, 10}, SkPoint{160, 150}, SkPoint{230, 10}});
    auto colors = std::to_array<SkColor>({
            0xff88ff00,
            0xff0088bb,
            0xff6600cc,
            0xffbb3377,
    });
    for (unsigned i = 0; i < std::size(colors); ++i) {
        paint.setColor(0x7fffffff & colors[i]);
        paint.setStrokeWidth(1);
        for (unsigned j = 0; j < 3; ++j) {
            canvas->drawLine(cubicPts[j], cubicPts[j + 1], paint);
        }
        SkPath path = SkPathBuilder()
                      .moveTo(cubicPts[0])
                      .cubicTo(cubicPts[1], cubicPts[2], cubicPts[3])
                      .detach();
        paint.setStrokeWidth(3);
        paint.setColor(colors[i]);
        canvas->drawPath(path, paint);
        cubicPts[1].fY += 30;
        cubicPts[2].fX += 30;
   }
}
}  // END FIDDLE
