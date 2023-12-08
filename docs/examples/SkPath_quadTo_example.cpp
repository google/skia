// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkPath_quadTo_example, 512, 512, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255, 255, 255, 255));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);

    SkPoint a{100, 100};
    SkPoint b{200, 400};
    SkPoint c{300, 100};

    SkPath twoSegments;
    twoSegments.moveTo(a);
    twoSegments.lineTo(b);
    twoSegments.lineTo(c);

    canvas->drawPath(twoSegments, paint);

    paint.setColor(SkColorSetARGB(255, 0, 0, 255));
    SkPath quadraticCurve;
    quadraticCurve.moveTo(a);
    quadraticCurve.quadTo(b, c);
    canvas->drawPath(quadraticCurve, paint);

    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 32);
    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    canvas->drawString("a", a.x(), a.y(), font, textPaint);
    canvas->drawString("b", b.x() + 20, b.y() + 20, font, textPaint);
    canvas->drawString("c", c.x(), c.y(), font, textPaint);
}
}  // END FIDDLE
