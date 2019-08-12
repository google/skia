/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"
#include "src/utils/SkUTF.h"

class EmbossView : public Sample {
    SkString name() override { return SkString("Emboss"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkEmbossMaskFilter::Light light = {{1, 1, 1}, 0, 128, 16 * 2};
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setMaskFilter(SkEmbossMaskFilter::Make(SkBlurMask::ConvertRadiusToSigma(4), light));
        paint.setShader(SkShaders::Color(SK_ColorBLUE));
        paint.setDither(true);

        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new EmbossView(); )
