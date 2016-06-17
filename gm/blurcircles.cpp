/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkString.h"

class BlurCirclesGM : public skiagm::GM {
public:
    BlurCirclesGM() { }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        return SkString("blurcircles");
    }

    SkISize onISize() override {
        return SkISize::Make(950, 950);
    }

    void onOnceBeforeDraw() override {
        const float blurRadii[kNumBlurs] = { 1,5,10,20 };

        for (int i = 0; i < kNumBlurs; ++i) {
            fBlurFilters[i] = SkBlurMaskFilter::Make(
                                    kNormal_SkBlurStyle,
                                    SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(blurRadii[i])),
                                    SkBlurMaskFilter::kHighQuality_BlurFlag);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(1.5f, 1.5f);
        canvas->translate(50,50);

        const int circleRadii[] = { 5,10,25,50 };

        for (size_t i = 0; i < kNumBlurs; ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, SkIntToScalar(150*i));
            for (size_t j = 0; j < SK_ARRAY_COUNT(circleRadii); ++j) {
                SkPaint paint;
                paint.setColor(SK_ColorBLACK);
                paint.setMaskFilter(fBlurFilters[i]);

                canvas->drawCircle(SkIntToScalar(50),SkIntToScalar(50),SkIntToScalar(circleRadii[j]),paint);
                canvas->translate(SkIntToScalar(150), 0);
            }
        }
    }
private:
    static const int kNumBlurs = 4;

    sk_sp<SkMaskFilter> fBlurFilters[kNumBlurs];

    typedef         skiagm::GM INHERITED;
};

DEF_GM(return new BlurCirclesGM();)
