// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(compose_path, 256, 256, false, 0) {
SkPath star() {
    const SkScalar R = 115.2f, C = 128.0f;
    SkPath path;
    path.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
        SkScalar a = 2.6927937f * i;
        path.lineTo(C + R * cos(a), C + R * sin(a));
    }
    return path;
}
void draw(SkCanvas* canvas) {
    const SkScalar intervals[] = {10.0f, 5.0f, 2.0f, 5.0f};
    SkPaint paint;
    paint.setPathEffect(
            SkPathEffect::MakeCompose(SkDashPathEffect::Make(intervals, 0.0f),
                                      SkDiscretePathEffect::Make(10.0f, 4.0f)));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.0f);
    paint.setAntiAlias(true);
    paint.setColor(0xff4285F4);
    canvas->clear(SK_ColorWHITE);
    SkPath path(star());
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
