// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(massive_coordinates_svg, 800, 600, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);

    SkRect r = {0, 0, 800, 590};
    paint.setColor(SK_ColorGREEN);
    canvas->drawRect(r, paint);

    canvas->clipRect(r);

    paint.setColor(SK_ColorBLACK);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    SkPath path;
    path.moveTo(-1000, 12345678901234567890.f);
    path.lineTo(200, 200);
    canvas->drawPath(path, paint);

    path.reset();
    path.moveTo(600, 400);
    path.lineTo(1000, -9.8765432109876543210e+19f);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
