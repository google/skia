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

class BlurRoundRectGM : public skiagm::GM {
public:
    BlurRoundRectGM(int width, int height,
                    // X and Y radii for the upper left corner
                    int ulX, int ulY,
                    // X and Y radii for the upper right corner
                    int urX, int urY,
                    // X and Y radii for the lower right corner
                    int lrX, int lrY,
                    // X and Y radii for the lower left corner
                    int llX, int llY,
                    int scaleX, int scaleY)
        : fName("blurroundrect")
        , fWidth(width)
        , fHeight(height)
        , fScaleX(scaleX)
        , fScaleY(scaleY) {
        fName.appendf("-WH[%ix%i]-UL[%ix%i]-UR[%ix%i]-LR[%ix%i]-LL[%ix%i]-scale[%ix%i]",
                      width,  height,
                      ulX,    ulY,
                      urX,    urY,
                      lrX,    lrY,
                      llX,    llY,
                      scaleX, scaleY);
        SkVector radii[4];
        radii[0].set(SkIntToScalar(ulX), SkIntToScalar(ulY));
        radii[1].set(SkIntToScalar(urX), SkIntToScalar(urY));
        radii[2].set(SkIntToScalar(lrX), SkIntToScalar(lrY));
        radii[3].set(SkIntToScalar(llX), SkIntToScalar(llY));
        SkRect r = SkRect::MakeWH(fWidth, fHeight);
        fRRect.setRectRadii(r, radii);
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        SkISize size = this->getUnscaledSize();
        return SkISize::Make(SkScalarCeilToInt(SkScalarMul(size.fWidth, fScaleX)),
                             SkScalarCeilToInt(SkScalarMul(size.fHeight, fScaleY)));
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->scale(fScaleX, fScaleY);
    }

    const SkRRect& getRRect() const {
        return fRRect;
    }

    // The subclass will implement this to inform us how big they
    // draw before scaling.
    virtual SkISize getUnscaledSize() const = 0;

    // So subclasses can modify the name.
    SkString* getName() {
        return &fName;
    }

private:
    SkString fName;
    const int fWidth;
    const int fHeight;
    const SkScalar fScaleX;
    const SkScalar fScaleY;
    SkRRect fRRect;
    typedef skiagm::GM INHERITED;
};

class SKPBlurRoundRectGM : public BlurRoundRectGM {
public:
    SKPBlurRoundRectGM(int width, int height,
                       int ulX, int ulY,
                       int urX, int urY,
                       int lrX, int lrY,
                       int llX, int llY,
                       int scaleX, int scaleY)
        : INHERITED(width, height, ulX, ulY, urX, urY, lrX, lrY, llX, llY, scaleX, scaleY) {
        this->getName()->prepend("skp-");
    }

protected:
    virtual SkISize getUnscaledSize() const SK_OVERRIDE {
        return SkISize::Make(SkScalarCeilToInt(this->getRRect().rect().width()),
                             SkScalarCeilToInt(this->getRRect().rect().height()));
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        this->INHERITED::onDraw(canvas);
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
            SkColorFilter* colorFilter = SkColorFilter::CreateModeFilter((SkColor) 4279308561,
                    SkXfermode::kSrcIn_Mode);
            paint->setColorFilter(colorFilter)->unref();
            paint->setColor((SkColor) 4278190080);
        }
        {
            SkLayerDrawLooper::LayerInfo info;
            looper->addLayerOnTop(info);
        }
        SkPaint paint;
        canvas->drawRect(this->getRRect().rect(), paint);

        paint.setLooper(looper)->unref();
        paint.setColor((SkColor) 4293848814);
        paint.setAntiAlias(true);

        canvas->drawRRect(this->getRRect(), paint);
    }

private:
    typedef BlurRoundRectGM INHERITED;
};

class SimpleBlurRoundRectGM : public BlurRoundRectGM {
public:
    SimpleBlurRoundRectGM(int width, int height,
                          int blurRadius, int cornerRadius,
                          int scaleX = 1, int scaleY = 1)
        : INHERITED(width, height, cornerRadius, cornerRadius,
                    cornerRadius, cornerRadius, cornerRadius,
                    cornerRadius, cornerRadius, cornerRadius, scaleX, scaleY)
        , fBlurRadius(blurRadius) {
        // For now at least, change the name to reflect only the
        // variables that are changing.
        this->getName()->printf("blurround-blur[%i]-corner[%i]-scale[%ix%i]", fBlurRadius, cornerRadius, scaleX, scaleY);
    }

protected:
    virtual SkISize getUnscaledSize() const SK_OVERRIDE {
        return SkISize::Make(SkScalarCeilToInt(this->getRRect().rect().width() + 20),
                             SkScalarCeilToInt(this->getRRect().rect().height() + 20));
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        // Handle the scaling.
        this->INHERITED::onDraw(canvas);
        canvas->translate(10, 10);
        SkMaskFilter* filter = SkBlurMaskFilter::Create(fBlurRadius,
                SkBlurMaskFilter::kNormal_BlurStyle);
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setMaskFilter(filter)->unref();
        canvas->drawRRect(this->getRRect(), paint);
    }
private:
    const int fBlurRadius;

    typedef BlurRoundRectGM INHERITED;
};

// Create one with dimensions/rounded corners based on the skp
DEF_GM(return new SKPBlurRoundRectGM(600, 5514, 6, 6, 6, 6, 6, 6, 6, 6, 1, 1);)
// Same radii, much smaller rectangle
DEF_GM(return new SKPBlurRoundRectGM(100, 100, 6, 6, 6, 6, 6, 6, 6, 6, 2, 2);)
// Rounded rect with two opposite corners with large radii, the other two
// small.
DEF_GM(return new SKPBlurRoundRectGM(100, 100, 30, 30, 10, 10, 30, 30, 10, 10, 3, 4);)
DEF_GM(return new SKPBlurRoundRectGM(100, 100, 90, 90, 90, 90, 90, 90, 90, 90, 2, 3);)

// Try a few blur values with a small corner radius
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 1, 1));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 3, 1, 2, 2));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 6, 1));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 10, 1, 3, 3));

// Now a few blur values with a larger corner radius
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 1, 3, 2, 2));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 3, 3));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 6, 3, 3, 3));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 10, 3));

// Even larger corner radius
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 1, 6, 2, 4));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 3, 6));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 6, 6));
DEF_GM(return new SimpleBlurRoundRectGM(100, 100, 10, 6, 1, 3));

