// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(gamma_mask_filter, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const char text[] = "Skia";
    canvas->drawColor(SK_ColorWHITE);

    SkFont font(fontMgr->matchFamilyStyle(nullptr, {}), 80);
    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->drawString(text, 16, 80, font, paint);

    paint.setMaskFilter(sk_sp<SkMaskFilter>(SkTableMaskFilter::CreateGamma(4.0f)));
    canvas->drawString(text, 16, 160, font, paint);

    paint.setMaskFilter(sk_sp<SkMaskFilter>(SkTableMaskFilter::CreateGamma(0.25f)));
    canvas->drawString(text, 16, 240, font, paint);
}
}  // END FIDDLE
