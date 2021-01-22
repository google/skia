// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_sum_path_effect, 256, 256, false, 0) {
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
    SkPaint paint;
    paint.setPathEffect(SkPathEffect::MakeSum(SkDiscretePathEffect::Make(10.0f, 4.0f),
                                              SkDiscretePathEffect::Make(10.0f, 4.0f, 1245u)));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.0f);
    paint.setAntiAlias(true);
    canvas->clear(SK_ColorWHITE);
    SkPath path(star());
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
