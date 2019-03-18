// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=84ec12a36e50df5ac565cc7a75ffbe9f
REG_FIDDLE(Draw_Looper_Methods, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkLayerDrawLooper::LayerInfo info;
    info.fPaintBits = (SkLayerDrawLooper::BitFlags) SkLayerDrawLooper::kColorFilter_Bit;
    info.fColorMode = SkBlendMode::kSrc;
    SkLayerDrawLooper::Builder looperBuilder;
    SkPaint* loopPaint = looperBuilder.addLayer(info);
    loopPaint->setColor(SK_ColorRED);
    info.fOffset.set(20, 20);
    loopPaint = looperBuilder.addLayer(info);
    loopPaint->setColor(SK_ColorBLUE);
    SkPaint paint;
    paint.setDrawLooper(looperBuilder.detach());
    canvas->drawCircle(50, 50, 50, paint);
}
}  // END FIDDLE
