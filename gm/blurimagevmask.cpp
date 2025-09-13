/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkImageFilters.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <stdio.h>

DEF_SIMPLE_GM(blurimagevmask, canvas, 700, 1200) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);

    SkFont font(ToolUtils::DefaultPortableTypeface(), 25);

    const double sigmas[] = {3.0, 8.0, 16.0, 24.0, 32.0};

    canvas->drawString("mask blur",  285, 50, font, paint);
    canvas->drawString("image blur", 285 + 250, 50, font, paint);


    SkRect r = {35, 100, 135, 200};
    for (auto sigma:sigmas) {

        canvas->drawRect(r, paint);

        char out[100];
        snprintf(out, std::size(out), "Sigma: %g", sigma);
        canvas->drawString(out, r.left(), r.bottom() + 35, font, paint);

        r.offset(250, 0);

        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
        canvas->drawRect(r, paint);
        paint.setMaskFilter(nullptr);

        SkPaint imageBlurPaint;
        r.offset(250, 0);
        imageBlurPaint.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));
        canvas->saveLayer(nullptr, &imageBlurPaint);

        canvas->drawRect(r, paint);
        canvas->restore();
        r.offset(-500, 200);
    }

}

DEF_SIMPLE_GM_CAN_FAIL(blur_image, canvas, errorMsg, 500, 500) {
    auto image = ToolUtils::GetResourceAsImage("images/mandrill_128.png");
    if (!image) {
        *errorMsg = "Could not load mandrill_128.png. Did you forget to set the resourcePath?";
        return skiagm::DrawResult::kFail;
    }

    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 4));

    // both of these should draw with the blur, but (formerally) we had a bug where the unscaled
    // version (taking the spriteblitter code path) ignore the maskfilter.

    canvas->drawImage(image, 10, 10, SkSamplingOptions(), &paint);
    canvas->scale(1.01f, 1.01f);
    canvas->drawImage(image, 10 + image->width() + 10.f, 10, SkSamplingOptions(), &paint);
    return skiagm::DrawResult::kOk;
}
