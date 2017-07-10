/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkBlurImageFilter.h"
#include "gm.h"

DEF_SIMPLE_GM(blurimagevmask, canvas, 700, 1200) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    const double sigmas[] = {3.0, 8.0, 16.0, 24.0, 32.0};

    SkRect r = {20, 100, 120, 200};
    for (auto sigma:sigmas) {

        canvas->drawRect(r, paint);

        r.offset(200, 0);

        paint.setMaskFilter(
            SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma,
                                   SkBlurMaskFilter::kHighQuality_BlurFlag));
        canvas->drawRect(r, paint);
        paint.setMaskFilter(nullptr);

        SkPaint imageBlurPaint;
        r.offset(200, 0);
        imageBlurPaint.setImageFilter(SkBlurImageFilter::Make(sigma, sigma, nullptr));
        canvas->saveLayer(nullptr, &imageBlurPaint);

        canvas->drawRect(r, paint);
        canvas->restore();
        r.offset(-400, 200);
    }

}
