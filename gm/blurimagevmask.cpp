/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkBlurImageFilter.h"
#include "gm.h"
#include "sk_tool_utils.h"


DEF_SIMPLE_GM(blurimagevmask, canvas, 700, 1200) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    SkPaint textPaint;
    textPaint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&textPaint);
    textPaint.setTextSize(SkIntToScalar(25));

    const double sigmas[] = {3.0, 8.0, 16.0, 24.0, 32.0};

    canvas->drawString("mask blur",  285, 50, textPaint);
    canvas->drawString("image blur", 285 + 250, 50, textPaint);


    SkRect r = {35, 100, 135, 200};
    for (auto sigma:sigmas) {

        canvas->drawRect(r, paint);

        char out[100];
        sprintf(out, "Sigma: %g", sigma);
        canvas->drawString(out, r.left(), r.bottom() + 35, textPaint);

        r.offset(250, 0);

        paint.setMaskFilter(
            SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma,
                                   SkBlurMaskFilter::kHighQuality_BlurFlag));
        canvas->drawRect(r, paint);
        paint.setMaskFilter(nullptr);

        SkPaint imageBlurPaint;
        r.offset(250, 0);
        imageBlurPaint.setImageFilter(SkBlurImageFilter::Make(sigma, sigma, nullptr));
        canvas->saveLayer(nullptr, &imageBlurPaint);

        canvas->drawRect(r, paint);
        canvas->restore();
        r.offset(-500, 200);
    }

}
