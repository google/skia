/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "src/core/SkBlurMask.h"
#include "src/effects/SkEmbossMaskFilter.h"

static SkBitmap make_bm() {
    SkBitmap bm;
    bm.allocN32Pixels(100, 100);

    SkCanvas canvas(bm);
    canvas.clear(0);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawCircle(50, 50, 50, paint);
    return bm;
}

class EmbossGM : public skiagm::GM {
public:
    EmbossGM() {
    }

protected:
    SkString onShortName() override {
        return SkString("emboss");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        SkBitmap bm = make_bm();
        canvas->drawBitmap(bm, 10, 10, &paint);
        canvas->translate(bm.width() + SkIntToScalar(10), 0);

        paint.setMaskFilter(SkEmbossMaskFilter::Make(
            SkBlurMask::ConvertRadiusToSigma(3),
            { { SK_Scalar1, SK_Scalar1, SK_Scalar1 }, 0, 128, 16*2 }));
        canvas->drawBitmap(bm, 10, 10, &paint);
        canvas->translate(bm.width() + SkIntToScalar(10), 0);

        // this combination of emboss+colorfilter used to crash -- so we exercise it to
        // confirm that we have a fix.
        paint.setColorFilter(SkColorFilters::Blend(0xFFFF0000, SkBlendMode::kSrcATop));
        canvas->drawBitmap(bm, 10, 10, &paint);
        canvas->translate(bm.width() + SkIntToScalar(10), 0);

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(10));
        paint.setMaskFilter(SkEmbossMaskFilter::Make(
            SkBlurMask::ConvertRadiusToSigma(4),
            { { SK_Scalar1, SK_Scalar1, SK_Scalar1 }, 0, 128, 16*2 }));
        paint.setColorFilter(nullptr);
        paint.setShader(SkShaders::Color(SK_ColorBLUE));
        paint.setDither(true);
        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);
        canvas->translate(SkIntToScalar(100), 0);

        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawString("Hello", 0, 50, SkFont(nullptr, 50), paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new EmbossGM;)
