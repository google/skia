/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkLayerDrawLooper.h"
#include "SkRandom.h"

#define WIDTH   200
#define HEIGHT  200

class DrawLooperGM : public skiagm::GM {
public:
    DrawLooperGM() : fLooper(NULL) {
        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~DrawLooperGM() {
        SkSafeUnref(fLooper);
    }

protected:
    virtual SkISize onISize() {
        return SkISize::Make(520, 160);
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("drawlooper");
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        this->init();

        SkPaint  paint;
        paint.setTextSize(SkIntToScalar(72));
        paint.setLooper(fLooper);

        canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                           SkIntToScalar(30), paint);

        canvas->drawRectCoords(SkIntToScalar(150), SkIntToScalar(50),
                               SkIntToScalar(200), SkIntToScalar(100), paint);

        canvas->drawText("Looper", 6, SkIntToScalar(230), SkIntToScalar(100),
                         paint);
    }

private:
    SkLayerDrawLooper*   fLooper;

    void init() {
        if (fLooper) return;

        static const struct {
            SkColor         fColor;
            SkPaint::Style  fStyle;
            SkScalar        fWidth;
            SkScalar        fOffset;
            SkScalar        fBlur;
        } gParams[] = {
            { SK_ColorWHITE, SkPaint::kStroke_Style, SkIntToScalar(1)*3/4, 0, 0 },
            { SK_ColorRED, SkPaint::kStroke_Style, SkIntToScalar(4), 0, 0 },
            { SK_ColorBLUE, SkPaint::kFill_Style, 0, 0, 0 },
            { 0x88000000, SkPaint::kFill_Style, 0, SkIntToScalar(10), SkIntToScalar(3) }
        };

        fLooper = new SkLayerDrawLooper;

        SkLayerDrawLooper::LayerInfo info;
        info.fFlagsMask = SkPaint::kAntiAlias_Flag;
        info.fPaintBits = SkLayerDrawLooper::kStyle_Bit | SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkXfermode::kSrc_Mode;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gParams); i++) {
            info.fOffset.set(gParams[i].fOffset, gParams[i].fOffset);
            SkPaint* paint = fLooper->addLayer(info);
            paint->setAntiAlias(true);
            paint->setColor(gParams[i].fColor);
            paint->setStyle(gParams[i].fStyle);
            paint->setStrokeWidth(gParams[i].fWidth);
            if (gParams[i].fBlur > 0) {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                                         SkBlurMask::ConvertRadiusToSigma(gParams[i].fBlur));
                paint->setMaskFilter(mf)->unref();
            }
        }
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new DrawLooperGM; }
static skiagm::GMRegistry reg(MyFactory);
