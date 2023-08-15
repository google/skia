/*
* Copyright 2014 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBlurMask.h"

class BlurCirclesGM : public skiagm::GM {
public:
    BlurCirclesGM() { }

protected:
    bool runAsBench() const override { return true; }

    SkString getName() const override { return SkString("blurcircles"); }

    SkISize getISize() override { return SkISize::Make(950, 950); }

    void onOnceBeforeDraw() override {
        const float blurRadii[kNumBlurs] = {1.f, 5.f, 10.f, 20.f};

        for (int i = 0; i < kNumBlurs; ++i) {
            fBlurFilters[i] = SkMaskFilter::MakeBlur(
                                    kNormal_SkBlurStyle,
                                    SkBlurMask::ConvertRadiusToSigma(blurRadii[i]));
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(1.5f, 1.5f);
        canvas->translate(50,50);

        const float circleRadii[] = {5.f, 10.f, 25.f, 50.f};

        for (size_t i = 0; i < kNumBlurs; ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, 150.f*i);
            for (size_t j = 0; j < std::size(circleRadii); ++j) {
                SkPaint paint;
                paint.setColor(SK_ColorBLACK);
                paint.setMaskFilter(fBlurFilters[i]);

                static constexpr SkPoint kCenter = {50.f, 50.f};
                // Throw a rotation in the mix to make sure GPU fast path handles it correctly.
                canvas->save();
                canvas->rotate(j*22.f, kCenter.fX, kCenter.fY);
                canvas->drawCircle(kCenter, circleRadii[j], paint);
                canvas->restore();
                canvas->translate(150.f, 0.f);
            }
        }
    }
private:
    inline static constexpr int kNumBlurs = 4;

    sk_sp<SkMaskFilter> fBlurFilters[kNumBlurs];

    using INHERITED =         skiagm::GM;
};

DEF_GM(return new BlurCirclesGM();)
