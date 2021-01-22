// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(kLow_SkFilterQuality, 192, 192, false, 0) {
void draw(SkCanvas* canvas) {
    static const uint32_t pixels[9] = {
            0xFFFF0000, 0xFFFFFFFF, 0xFF00FF00, 0xFFFFFFFF, 0xFF000000,
            0xFFFFFFFF, 0xFF808080, 0xFFFFFFFF, 0xFF0000FF,
    };
    SkBitmap bm;
    bm.installPixels(SkImageInfo::MakeN32Premul(3, 3), (void*)pixels, 12);
    bm.setImmutable();
    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    // paint.setMaskFilter(SkBlurMaskFilter::Make(
    //            kNormal_SkBlurStyle, 3.375, SkBlurMaskFilter::kHighQuality_BlurFlag));
    canvas->drawBitmapRect(bm, {64, 64, 128, 128}, &paint);
}
}  // END FIDDLE
