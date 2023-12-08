// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=55f5e59350622c5e2834d1c85789f732
REG_FIDDLE(Canvas_drawText, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkFont defaultFont = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));
    float textSizes[] = { 12, 18, 24, 36 };
    for (auto size: textSizes ) {
        defaultFont.setSize(size);
        canvas->drawString("Aa", 10, 20, defaultFont, paint);
        canvas->translate(0, size * 2);
    }
    defaultFont = SkFont(fontMgr->matchFamilyStyle(nullptr, {}));
    float yPos = 20;
    for (auto size: textSizes ) {
        float scale = size / 12.f;
        canvas->resetMatrix();
        canvas->translate(100, 0);
        canvas->scale(scale, scale);
        canvas->drawString("Aa", 10 / scale, yPos / scale, defaultFont, paint);
        yPos += size * 2;
    }
}
}  // END FIDDLE
