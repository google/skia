/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

class DrawRegionModesGM : public skiagm::GM {
public:
    DrawRegionModesGM() {}

protected:
    SkString onShortName() override {
        return SkString("drawregionmodes");
    }

    SkISize onISize() override {
        return SkISize::Make(375, 500);
    }

    void onOnceBeforeDraw() override {
        fRegion.op({50,  50, 100, 100}, SkRegion::kUnion_Op);
        fRegion.op({50, 100, 150, 150}, SkRegion::kUnion_Op);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGREEN);

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);

        canvas->save();
        canvas->translate(-50.0f, 75.0f);
        canvas->rotate(-45.0f);
        canvas->drawRegion(fRegion, paint);

        canvas->translate(125.0f, 125.0f);
        paint.setImageFilter(SkImageFilters::Blur(5.0f, 5.0f, nullptr, nullptr));
        canvas->drawRegion(fRegion, paint);

        canvas->translate(-125.0f, 125.0f);
        paint.setImageFilter(nullptr);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0f));
        canvas->drawRegion(fRegion, paint);

        canvas->translate(-125.0f, -125.0f);
        paint.setMaskFilter(nullptr);
        paint.setStyle(SkPaint::kStroke_Style);
        float intervals[] = { 5.0f, 5.0f };
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));
        canvas->drawRegion(fRegion, paint);

        canvas->restore();

        canvas->translate(100, 325);
        paint.setPathEffect(nullptr);
        paint.setStyle(SkPaint::kFill_Style);
        SkPoint points[] = { SkPoint::Make(50.0f, 50.0f), SkPoint::Make(150.0f, 150.0f) };
        SkColor colors[] = { SK_ColorBLUE, SK_ColorYELLOW };
        paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 2,
                                                     SkTileMode::kClamp));
        canvas->drawRegion(fRegion, paint);
    }

private:
    SkRegion fRegion;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new DrawRegionModesGM; )
