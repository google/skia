// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(measure_text_bounds, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setAntiAlias(true);
    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 64, 1.25f, -0.25f);

    float x = 32.0f;
    float y = 88.0f;
    const char text[] = "Skia";

    SkRect bounds;
    (void)font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);

    canvas->drawRect(bounds.makeOffset(x, y), paint);

    paint.setColor(SK_ColorWHITE);
    canvas->drawString(text, x, y, font, paint);
}
}  // END FIDDLE
