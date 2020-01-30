// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(maddash, 400, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(380);

    SkScalar intvls[] = {2.5, 10000};
    p.setPathEffect(SkDashPathEffect::Make(intvls, 2, 0));

    canvas->drawCircle(0, 100, 200, p);
}
}  // END FIDDLE
