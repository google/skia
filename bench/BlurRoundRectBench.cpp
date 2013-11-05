/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkBenchmark.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkLayerDrawLooper.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkRRect.h"
#include "SkString.h"
#include "SkXfermode.h"

class BlurRoundRectBench : public SkBenchmark {
public:
    BlurRoundRectBench(int width, int height,
                       // X and Y radii for the upper left corner
                       int ulX, int ulY,
                       // X and Y radii for the upper right corner
                       int urX, int urY,
                       // X and Y radii for the lower right corner
                       int lrX, int lrY,
                       // X and Y radii for the lower left corner
                       int llX, int llY)
        : fName("blurroundrect")
        , fWidth(width)
        , fHeight(height) {
        fName.appendf("_WH[%ix%i]_UL[%ix%i]_UR[%ix%i]_LR[%ix%i]_LL[%ix%i]",
                      width, height,
                      ulX,   ulY,
                      urX,   urY,
                      lrX,   lrY,
                      llX,   llY);
        fRadii[0].set(SkIntToScalar(ulX), SkIntToScalar(ulY));
        fRadii[1].set(SkIntToScalar(urX), SkIntToScalar(urY));
        fRadii[2].set(SkIntToScalar(lrX), SkIntToScalar(lrY));
        fRadii[3].set(SkIntToScalar(llX), SkIntToScalar(llY));
    }

    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual SkIPoint onGetSize() SK_OVERRIDE {
        return SkIPoint::Make(fWidth, fHeight);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        for (int i = 0; i < this->getLoops(); i++) {
            SkLayerDrawLooper* looper = new SkLayerDrawLooper;
            {
                SkLayerDrawLooper::LayerInfo info;
                info.fFlagsMask = 0;
                info.fPaintBits = 40;
                info.fColorMode = SkXfermode::kSrc_Mode;
                info.fOffset = SkPoint::Make(SkIntToScalar(-1), SkIntToScalar(0));
                info.fPostTranslate = false;
                SkPaint* paint = looper->addLayerOnTop(info);
                SkMaskFilter* maskFilter = SkBlurMaskFilter::Create(SK_ScalarHalf,
                        SkBlurMaskFilter::kNormal_BlurStyle,
                        SkBlurMaskFilter::kHighQuality_BlurFlag);
                paint->setMaskFilter(maskFilter)->unref();
                SkColorFilter* colorFilter = SkColorFilter::CreateModeFilter(SK_ColorLTGRAY,
                        SkXfermode::kSrcIn_Mode);
                paint->setColorFilter(colorFilter)->unref();
                paint->setColor(SK_ColorGRAY);
            }
            {
                SkLayerDrawLooper::LayerInfo info;
                looper->addLayerOnTop(info);
            }
            SkPaint paint;
            SkRect rect = SkRect::MakeWH(SkIntToScalar(fWidth), SkIntToScalar(fHeight));
            canvas->drawRect(rect, paint);

            paint.setLooper(looper)->unref();
            paint.setAntiAlias(true);
            paint.setColor(SK_ColorCYAN);

            SkRRect rrect;
            rrect.setRectRadii(rect, fRadii);
            canvas->drawRRect(rrect, paint);
        }
    }

private:
    SkString fName;
    const int fWidth;
    const int fHeight;
    SkVector fRadii[4];
    typedef SkBenchmark INHERITED;
};

// Create one with dimensions/rounded corners based on the skp
DEF_BENCH(return new BlurRoundRectBench(600, 5514, 6, 6, 6, 6, 6, 6, 6, 6);)
// Same radii, much smaller rectangle
DEF_BENCH(return new BlurRoundRectBench(100, 100, 6, 6, 6, 6, 6, 6, 6, 6);)
// Rounded rect with two opposite corners with large radii, the other two
// small.
DEF_BENCH(return new BlurRoundRectBench(100, 100, 30, 30, 10, 10, 30, 30, 10, 10);)
DEF_BENCH(return new BlurRoundRectBench(100, 100, 90, 90, 90, 90, 90, 90, 90, 90);)
