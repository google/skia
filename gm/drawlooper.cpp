/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkLayerDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkRandom.h"
#include "ToolUtils.h"
#include "gm.h"

#define WIDTH   200
#define HEIGHT  200

class DrawLooperGM : public skiagm::GM {
public:
    DrawLooperGM() : fLooper(nullptr) {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual SkISize onISize() override {
        return SkISize::Make(520, 160);
    }

    SkString onShortName() override {
        return SkString("drawlooper");
    }

    void onDraw(SkCanvas* canvas) override {
        this->init();

        SkPaint  paint;
        paint.setAntiAlias(true);
        paint.setLooper(fLooper);

        SkFont font(ToolUtils::create_portable_typeface(), 72);

        canvas->drawCircle(50, 50, 30, paint);
        canvas->drawRect({ 150, 50, 200, 100 }, paint);
        canvas->drawString("Looper", 230, 100, font, paint);
    }

private:
    sk_sp<SkDrawLooper> fLooper;

    void init() {
        if (fLooper) return;

        constexpr struct {
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

        SkLayerDrawLooper::Builder looperBuilder;

        SkLayerDrawLooper::LayerInfo info;
        info.fPaintBits = SkLayerDrawLooper::kStyle_Bit | SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkBlendMode::kSrc;

        for (size_t i = 0; i < SK_ARRAY_COUNT(gParams); i++) {
            info.fOffset.set(gParams[i].fOffset, gParams[i].fOffset);
            SkPaint* paint = looperBuilder.addLayer(info);
            paint->setColor(gParams[i].fColor);
            paint->setStyle(gParams[i].fStyle);
            paint->setStrokeWidth(gParams[i].fWidth);
            if (gParams[i].fBlur > 0) {
                paint->setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                         SkBlurMask::ConvertRadiusToSigma(gParams[i].fBlur)));
            }
        }
        fLooper = looperBuilder.detach();
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DrawLooperGM; )
