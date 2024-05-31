// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_saveLayer_4, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkPaint pRed;
    pRed.setColor(SK_ColorRED);

    SkPaint pSolidBlue;
    pSolidBlue.setColor(SK_ColorBLUE);

    SkPaint pThirtyBlue;
    pThirtyBlue.setColor(SK_ColorBLUE);
    pThirtyBlue.setAlphaf(0.3);

    SkPaint alpha;
    alpha.setAlphaf(0.3);

    // First row: Draw two opaque red rectangles into the 0th layer. Then draw two blue
    // rectangles overlapping the red, one is solid, the other is 30% transparent.
    canvas->drawRect(SkRect::MakeLTRB(10, 10, 60, 60), pRed);
    canvas->drawRect(SkRect::MakeLTRB(150, 10, 200, 60), pRed);

    canvas->drawRect(SkRect::MakeLTRB(30, 10, 80, 60), pSolidBlue);
    canvas->drawRect(SkRect::MakeLTRB(170, 10, 220, 60), pThirtyBlue);

    // Second row: Draw two opaque red rectangles into the 0th layer. Then save a new layer;
    // when the 1st layer gets merged onto the 0th layer (i.e. when restore() is called), it will
    // use the provided paint to do so. In this case, the paint is set to have 30% opacity, but
    // it could also have things set like blend modes or image filters.
    canvas->drawRect(SkRect::MakeLTRB(10, 70, 60, 120), pRed);
    canvas->drawRect(SkRect::MakeLTRB(150, 70, 200, 120), pRed);

    canvas->saveLayer(nullptr, &alpha);

    // In the 1st layer, draw the same blue overlapping rectangles as in the first row. Notice in
    // the final output, we have two different shades of purple. The layer's alpha made the
    // opaque blue rectangle transparent, and it made the transparent blue rectangle even more so
    canvas->drawRect(SkRect::MakeLTRB(30, 70, 80, 120), pSolidBlue);
    canvas->drawRect(SkRect::MakeLTRB(170, 70, 220, 120), pThirtyBlue);

    canvas->restore();

    // Third row: save the layer first, before drawing the two red rectangle, followed by the
    // overlapping blue rectangles. Notice that the blue overwrites the red in the same way as
    // the first row because the alpha of the layer is not applied until the layer is restored.
    canvas->saveLayer(nullptr, &alpha);

    canvas->drawRect(SkRect::MakeLTRB(10, 130, 60, 180), pRed);
    canvas->drawRect(SkRect::MakeLTRB(150, 130, 200, 180), pRed);

    canvas->drawRect(SkRect::MakeLTRB(30, 130, 80, 180), pSolidBlue);
    canvas->drawRect(SkRect::MakeLTRB(170, 130, 220, 180), pThirtyBlue);

    canvas->restore();
}
}  // END FIDDLE
