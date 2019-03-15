// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=888edd4c4a91ca62ceb01bce8ab675b2
REG_FIDDLE(Path_092, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRRect rrect;
    rrect.setRectXY({40, 40, 215, 215}, 50, 50);
    SkPath path;
    path.addRRect(rrect);
    canvas->drawPath(path, paint);
    for (int start = 0; start < 8; ++start) {
        SkPath textPath;
        textPath.addRRect(rrect, SkPath::kCW_Direction, start);
        SkPathMeasure pathMeasure(textPath, false);
        SkPoint position;
        SkVector tangent;
        if (!pathMeasure.getPosTan(0, &position, &tangent)) {
            continue;
        }
        SkRSXform rsxForm = SkRSXform::Make(tangent.fX, tangent.fY,
               position.fX + tangent.fY * 5, position.fY - tangent.fX * 5);
        SkFont font(nullptr, 12);
        auto labels = SkTextBlob::MakeFromRSXform(&"01234567"[start], 1, &rsxForm, font);
        canvas->drawTextBlob(labels, 0, 0, paint);
    }
}
}  // END FIDDLE
