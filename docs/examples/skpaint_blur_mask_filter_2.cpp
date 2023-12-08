// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_blur_mask_filter_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const SkScalar blurSigma = 3.4f;
    const SkColor blurColor = SkColorSetRGB(96, 96, 0);
    const uint8_t blurAlpha = 127;
    const SkScalar xDrop = 5.0f;
    const SkScalar yDrop = 5.0f;

    const char text[] = "Skia";
    const SkScalar x = 0.0f;
    const SkScalar y = 160.0f;

    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkPaint blur(paint);
    blur.setColor(blurColor);
    blur.setAlpha(blurAlpha);
    blur.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, blurSigma, false));

    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 120);

    // Draw once with drop shadow blur;
    canvas->drawString(text, x + xDrop, y + yDrop, font, blur);

    // Overdraw with with no blur mask
    canvas->drawString(text, x, y, font, paint);
}
}  // END FIDDLE
