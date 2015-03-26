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
    BlurCirclesGM()
        : fName("blurcircles") {
    }

protected:

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(950, 950);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(1.5f, 1.5f);
        canvas->translate(50,50);

        const float blurRadii[] = { 1,5,10,20 };
        const int circleRadii[] = { 5,10,25,50 };
        for (size_t i = 0; i < SK_ARRAY_COUNT(blurRadii); ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, SkIntToScalar(150*i));
            for (size_t j = 0; j < SK_ARRAY_COUNT(circleRadii); ++j) {
                SkMaskFilter* filter = SkBlurMaskFilter::Create(
                    kNormal_SkBlurStyle,
                    SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(blurRadii[i])),
                    SkBlurMaskFilter::kHighQuality_BlurFlag);
                SkPaint paint;
                paint.setColor(SK_ColorBLACK);
                paint.setMaskFilter(filter)->unref();

                canvas->drawCircle(SkIntToScalar(50),SkIntToScalar(50),SkIntToScalar(circleRadii[j]),paint);
                canvas->translate(SkIntToScalar(150), 0);
            }
        }
    }
private:
    const SkString  fName;

    typedef         skiagm::GM INHERITED;
};

DEF_GM(return new BlurCirclesGM();)
