// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(not_path_gradient, 256, 256, false, 0) {
SkPath star() {
    const SkScalar R = 60.0f, C = 128.0f;
    SkPath path;
    path.moveTo(C + R, C);
    for (int i = 1; i < 15; ++i) {
        SkScalar a = 0.44879895f * i;
        SkScalar r = R + R * (i % 2);
        path.lineTo(C + r * cos(a), C + r * sin(a));
    }
    return path;
}
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setPathEffect(SkDiscretePathEffect::Make(10.0f, 4.0f));
    SkPoint points[2] = {SkPoint::Make(0.0f, 0.0f), SkPoint::Make(256.0f, 256.0f)};
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    SkColor colors[2] = {SkColorSetRGB(0xEA, 0xD2, 0xAC), SkColorSetRGB(0x42, 0x81, 0xA4)};
    paint.setShader(SkGradientShader::MakeLinear(
            points, colors, nullptr, 2, SkTileMode::kClamp, 0, nullptr));
    paint.setAntiAlias(true);
    canvas->clear(SK_ColorWHITE);
    SkPath path(star());
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
