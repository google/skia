/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkGradientShader.h"
#include "SkRegion.h"

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
        fRegion.op( 50,  50, 100, 100, SkRegion::kUnion_Op);
        fRegion.op( 50, 100, 150, 150, SkRegion::kUnion_Op);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGREEN);

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(0xFFFF0000);
        paint.setAntiAlias(true);

        canvas->translate(-50.0f, 75.0f);
        canvas->rotate(-45.0f);
        canvas->drawRegion(fRegion, paint);

        canvas->translate(125.0f, 125.0f);
        paint.setImageFilter(SkBlurImageFilter::Make(5.0f, 5.0f, nullptr, nullptr));
        canvas->drawRegion(fRegion, paint);

        canvas->translate(-125.0f, 125.0f);
        paint.setImageFilter(nullptr);
        SkRect occluder = SkRect::MakeEmpty();
        paint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 5.0f, occluder, 0));
        canvas->drawRegion(fRegion, paint);

        canvas->translate(-125.0f, -125.0f);
        paint.setMaskFilter(nullptr);
        paint.setStyle(SkPaint::kStroke_Style);
        float intervals[] = { 5.0f, 5.0f };
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));
        canvas->drawRegion(fRegion, paint);

        canvas->setMatrix(SkMatrix::I());
        canvas->translate(100, 325);
        paint.setPathEffect(nullptr);
        paint.setStyle(SkPaint::kFill_Style);
        SkPoint points[] = { SkPoint::Make(50.0f, 50.0f), SkPoint::Make(150.0f, 150.0f) };
        SkColor colors[] = { SK_ColorBLUE, SK_ColorYELLOW };
        paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 2,
                                                     SkShader::kClamp_TileMode));
        canvas->drawRegion(fRegion, paint);
    }

    SkRegion fRegion;

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new DrawRegionModesGM; )
