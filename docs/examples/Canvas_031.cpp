#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8460bf8b013f46c67e0bd96e13451aff
REG_FIDDLE(Canvas_031, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setLCDRenderText(true);
    paint.setTextSize(20);
    for (auto preserve : { false, true } ) {
        preserve ? canvas->saveLayerPreserveLCDTextRequests(nullptr, nullptr)
                 : canvas->saveLayer(nullptr, nullptr);
        SkPaint p;
        p.setColor(SK_ColorWHITE);
        // Comment out the next line to draw on a non-opaque background.
        canvas->drawRect(SkRect::MakeLTRB(25, 40, 200, 70), p);
        canvas->drawString("Hamburgefons", 30, 60, paint);
        p.setColor(0xFFCCCCCC);
        canvas->drawRect(SkRect::MakeLTRB(25, 70, 200, 100), p);
        canvas->drawString("Hamburgefons", 30, 90, paint);
        canvas->restore();
        canvas->translate(0, 80);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
