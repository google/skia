/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkMaskFilter.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"

SkScalar get_anim_sin(double secs, SkScalar amplitude, SkScalar periodInSec, SkScalar phaseInSec) {
    if (!periodInSec) {
        return 0;
    }
    double t = secs + phaseInSec;
    t *= SkScalarToFloat(2 * SK_ScalarPI) / periodInSec;
    amplitude = SK_ScalarHalf * amplitude;
    return amplitude * SkDoubleToScalar(sin(t)) + amplitude;
}

class AnimBlurView : public Sample {
    SkScalar fBlurSigma = 0;
    SkScalar fCircleRadius = 100;

    SkString name() override { return SkString("AnimBlur"); }

    void onDrawContent(SkCanvas* canvas) override {
        static const SkBlurStyle gStyles[] = {
            kNormal_SkBlurStyle,
            kInner_SkBlurStyle,
            kSolid_SkBlurStyle,
            kOuter_SkBlurStyle,
        };
        SkRandom random;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gStyles); ++i) {
            SkPaint paint;
            paint.setMaskFilter(SkMaskFilter::MakeBlur(gStyles[i],
                                                       fBlurSigma));
            paint.setColor(random.nextU() | 0xff000000);
            canvas->drawCircle(200 * SK_Scalar1 + 400 * (i % 2) * SK_Scalar1,
                               200 * SK_Scalar1 + i / 2 * 400 * SK_Scalar1,
                               fCircleRadius, paint);
        }
    }

    bool onAnimate(double nanos) override {
        fBlurSigma = get_anim_sin(1e-9 * nanos, 100, 4, 5);
        fCircleRadius = 3 + get_anim_sin(1e-9 * nanos, 150, 25, 3);
        return true;
    }
};
DEF_SAMPLE( return new AnimBlurView(); )
