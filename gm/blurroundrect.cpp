/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"
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

// This GM mimics a blurred RR seen in the wild.
class BlurRoundRectGM : public skiagm::GM {
public:
    BlurRoundRectGM(int width, int height, int radius)
        : fName("blurroundrect")
    {
        SkRect r = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
        fRRect.setRectXY(r, SkIntToScalar(radius), SkIntToScalar(radius));
        fName.appendf("-WH[%ix%i]-corner[%i]", width, height, radius);
    }

    BlurRoundRectGM(int width, int height)
        : fName("blurroundrect") {
        fName.appendf("-WH[%ix%i]-unevenCorners",
                      width,  height);
        SkVector radii[4];
        radii[0].set(SkIntToScalar(30), SkIntToScalar(30));
        radii[1].set(SkIntToScalar(10), SkIntToScalar(10));
        radii[2].set(SkIntToScalar(30), SkIntToScalar(30));
        radii[3].set(SkIntToScalar(10), SkIntToScalar(10));
        SkRect r = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));
        fRRect.setRectRadii(r, radii);
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(SkScalarCeilToInt(fRRect.rect().width()),
                             SkScalarCeilToInt(fRRect.rect().height()));
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkLayerDrawLooper* looper = new SkLayerDrawLooper;
        {
            SkLayerDrawLooper::LayerInfo info;
            info.fFlagsMask = 0;
            info.fPaintBits = SkLayerDrawLooper::kMaskFilter_Bit
                              | SkLayerDrawLooper::kColorFilter_Bit;
            info.fColorMode = SkXfermode::kSrc_Mode;
            info.fOffset = SkPoint::Make(SkIntToScalar(-1), SkIntToScalar(0));
            info.fPostTranslate = false;
            SkPaint* paint = looper->addLayerOnTop(info);
            SkMaskFilter* maskFilter = SkBlurMaskFilter::Create(
                    SkBlurMaskFilter::kNormal_BlurStyle,
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
            looper->addLayerOnTop(info);
        }
        SkPaint paint;
        canvas->drawRect(fRRect.rect(), paint);

        paint.setLooper(looper)->unref();
        paint.setColor(SK_ColorCYAN);
        paint.setAntiAlias(true);

        canvas->drawRRect(fRRect, paint);
    }

private:
    SkString        fName;
    SkRRect         fRRect;

    typedef skiagm::GM INHERITED;
};

// Simpler blurred RR test cases where all the radii are the same.
class SimpleBlurRoundRectGM : public skiagm::GM {
public:
    SimpleBlurRoundRectGM()
        : fName("simpleblurroundrect") {
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(750, 750);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->scale(1.5f, 1.5f);

        const int blurRadii[] = { 1, 3, 6, 10 };
        const int cornerRadii[] = { 1, 3, 6, 10 };
        const SkRect r = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));
        for (size_t i = 0; i < SK_ARRAY_COUNT(blurRadii); ++i) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, (r.height() + SkIntToScalar(20)) * i);
            for (size_t j = 0; j < SK_ARRAY_COUNT(cornerRadii); ++j) {
                SkMaskFilter* filter = SkBlurMaskFilter::Create(
                    SkBlurMaskFilter::kNormal_BlurStyle,
                    SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(blurRadii[i])));
                SkPaint paint;
                paint.setColor(SK_ColorBLUE);
                paint.setMaskFilter(filter)->unref();

                SkRRect rrect;
                rrect.setRectXY(r, SkIntToScalar(cornerRadii[j]), SkIntToScalar(cornerRadii[j]));
                canvas->drawRRect(rrect, paint);
                canvas->translate(r.width() + SkIntToScalar(10), 0);
            }
        }
    }
private:
    const SkString  fName;

    typedef         skiagm::GM INHERITED;
};

// Create one with dimensions/rounded corners based on the skp
//
// TODO(scroggo): Disabled in an attempt to rememdy
// https://code.google.com/p/skia/issues/detail?id=1801 ('Win7 Test bots all failing GenerateGMs:
// ran wrong number of tests')
//DEF_GM(return new BlurRoundRectGM(600, 5514, 6);)

// Rounded rect with two opposite corners with large radii, the other two
// small.
DEF_GM(return new BlurRoundRectGM(100, 100);)

DEF_GM(return new SimpleBlurRoundRectGM();)
