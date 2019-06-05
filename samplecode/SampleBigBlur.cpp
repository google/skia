/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"

static void big_blur(SkCanvas* canvas, SkSize) {
        SkPaint paint;
        canvas->save();
        paint.setColor(SK_ColorBLUE);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(
            kNormal_SkBlurStyle,
            SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(128))));
        canvas->translate(200, 200);
        canvas->drawCircle(100, 100, 200, paint);
        canvas->restore();
}
DEF_SIMPLE_SAMPLE("BigBlur", big_blur);
