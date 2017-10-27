/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkColor.h"

// GM to check the behavior from chrome bug:745290
DEF_SIMPLE_GM(blurSmallRadii, canvas, 100, 335) {
    double sigmas[] = {0.0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5};
    SkPaint paint;

    for (auto sigma : sigmas) {
        paint.setColor(SK_ColorBLACK);
        paint.setAntiAlias(true);
        paint.setTextSize(24);
        if (sigma != 0.0) {
            paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma));
        }
        canvas->drawString("Guest", 20, 24, paint);

        paint.setMaskFilter(nullptr);
        paint.setColor(SK_ColorWHITE);
        canvas->drawString("Guest", 20, 24, paint);

        canvas->translate(0, 30);
    }
}
