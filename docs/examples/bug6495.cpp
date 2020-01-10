// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(bug6495, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    SkRect r = SkRect::MakeXYWH(20, 20, 100, 100);

    canvas->rotate(90);
    canvas->scale(1.0, -1.0);
    canvas->drawOval(r, p);

    p.setColor(SK_ColorGREEN);

    canvas->save();
    canvas->scale(1.0, 0.4999);
    canvas->drawOval(r, p);
    canvas->restore();

    canvas->save();
    canvas->scale(1.0, 0.5000);
    canvas->drawOval(r, p);
    canvas->restore();

    canvas->save();
    canvas->scale(1.0, 0.5001);
    canvas->drawOval(r, p);
    canvas->restore();
}
}  // END FIDDLE
