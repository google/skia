// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_concat, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 80);
    font.setScaleX(.3f);
    SkRect rect[2] = {{ 10, 20, 90, 110 }, { 40, 130, 140, 180 }};
    canvas->drawRect(rect[0], paint);
    canvas->drawRect(rect[1], paint);
    paint.setColor(SK_ColorWHITE);
    canvas->drawString("Here", rect[0].fLeft + 10, rect[0].fBottom - 10, font, paint);
    canvas->concat(SkMatrix::Rect2Rect(rect[0], rect[1]).value());
    canvas->drawString("There", rect[0].fLeft + 10, rect[0].fBottom - 10, font, paint);
}
}  // END FIDDLE
