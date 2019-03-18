// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=85a76163138a2720ac003691d6363938
REG_FIDDLE(Image_makeWithFilter, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    sk_sp<SkImageFilter> shadowFilter = SkDropShadowImageFilter::Make(
                -10.0f * frame, 5.0f * frame, 3.0f, 3.0f, SK_ColorBLUE,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                nullptr);
    sk_sp<SkImageFilter> offsetFilter = SkOffsetImageFilter::Make(40, 40, shadowFilter, nullptr);
    SkIRect subset = image->bounds();
    SkIRect clipBounds = image->bounds();
    clipBounds.outset(60, 60);
    SkIRect outSubset;
    SkIPoint offset;
    sk_sp<SkImage> filtered(image->makeWithFilter(offsetFilter.get(), subset, clipBounds,
                            &outSubset, &offset));
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawLine(0, 0, offset.fX, offset.fY, paint);
    canvas->translate(offset.fX, offset.fY);
    canvas->drawImage(filtered, 0, 0);
    canvas->drawRect(SkRect::Make(outSubset), paint);
}
}  // END FIDDLE
