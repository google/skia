/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Benchmark.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkLayerDrawLooper.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkString.h"
#include "SkXfermode.h"

// Large blurred RR appear frequently on web pages. This benchmark measures our
// performance in this case.
class BlurRoundRectBench : public Benchmark {
public:
    BlurRoundRectBench(int width, int height, int cornerRadius)
        : fName("blurroundrect") {
        fName.appendf("_WH[%ix%i]_cr[%i]", width, height, cornerRadius);
        SkRect r = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
        fRRect.setRectXY(r, SkIntToScalar(cornerRadius), SkIntToScalar(cornerRadius));
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(SkScalarCeilToInt(fRRect.rect().width()),
                              SkScalarCeilToInt(fRRect.rect().height()));
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkLayerDrawLooper::Builder looperBuilder;
        {
            SkLayerDrawLooper::LayerInfo info;
            info.fPaintBits = SkLayerDrawLooper::kMaskFilter_Bit
                              | SkLayerDrawLooper::kColorFilter_Bit;
            info.fColorMode = SkXfermode::kSrc_Mode;
            info.fOffset = SkPoint::Make(SkIntToScalar(-1), SkIntToScalar(0));
            info.fPostTranslate = false;
            SkPaint* paint = looperBuilder.addLayerOnTop(info);
            SkMaskFilter* maskFilter = SkBlurMaskFilter::Create(
                    kNormal_SkBlurStyle,
                    SkBlurMask::ConvertRadiusToSigma(SK_ScalarHalf),
                    SkBlurMaskFilter::kHighQuality_BlurFlag);
            paint->setMaskFilter(maskFilter)->unref();
            SkColorFilter* colorFilter = SkColorFilter::CreateModeFilter(SK_ColorLTGRAY,
                    SkXfermode::kSrcIn_Mode);
            paint->setColorFilter(colorFilter)->unref();
            paint->setColor(SK_ColorGRAY);
        }
        {
            SkLayerDrawLooper::LayerInfo info;
            looperBuilder.addLayerOnTop(info);
        }
        SkPaint dullPaint;
        dullPaint.setAntiAlias(true);

        SkPaint loopedPaint;
        loopedPaint.setLooper(looperBuilder.detachLooper())->unref();
        loopedPaint.setAntiAlias(true);
        loopedPaint.setColor(SK_ColorCYAN);

        for (int i = 0; i < loops; i++) {
            canvas->drawRect(fRRect.rect(), dullPaint);
            canvas->drawRRect(fRRect, loopedPaint);
        }
    }

private:
    SkString    fName;
    SkRRect     fRRect;

    typedef     Benchmark INHERITED;
};

// Create one with dimensions/rounded corners based on the skp
DEF_BENCH(return new BlurRoundRectBench(600, 5514, 6);)
// Same radii, much smaller rectangle
DEF_BENCH(return new BlurRoundRectBench(100, 100, 6);)
// Other radii options
DEF_BENCH(return new BlurRoundRectBench(100, 100, 30);)
DEF_BENCH(return new BlurRoundRectBench(100, 100, 90);)
