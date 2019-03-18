// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7d3751e82d1b6ec328ffa3d6f48ca831
REG_FIDDLE(Canvas_saveLayer_3, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    // sk_sp<SkImage> image = GetResourceAsImage("images/mandrill_256.png");
    canvas->drawImage(image, 0, 0, nullptr);
    SkCanvas::SaveLayerRec rec;
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kPlus);
    rec.fSaveLayerFlags = SkCanvas::kInitWithPrevious_SaveLayerFlag;
    rec.fPaint = &paint;
    canvas->saveLayer(rec);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas->drawCircle(128, 128, 96, paint);
    canvas->restore();
}
}  // END FIDDLE
