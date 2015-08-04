/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurMask.h"
#include "SkPath.h"

namespace skiagm {

///////////////////////////////////////////////////////////////////////////////

static void setup(SkPaint* paint, SkColor c, SkScalar strokeWidth) {
    paint->setColor(c);
    if (strokeWidth < 0) {
        paint->setStyle(SkPaint::kFill_Style);
    } else {
        paint->setStyle(SkPaint::kStroke_Style);
        paint->setStrokeWidth(strokeWidth);
    }
}

class ShadowsGM : public GM {
public:
    SkPath fCirclePath;
    SkRect fRect;

protected:
    void onOnceBeforeDraw() override {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
        fCirclePath.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(10) );
        fRect.set(SkIntToScalar(10), SkIntToScalar(10),
                  SkIntToScalar(30), SkIntToScalar(30));
    }

    SkString onShortName() override {
        return SkString("shadows");
    }

    SkISize onISize() override {
        return SkISize::Make(200, 120);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBlurDrawLooper* shadowLoopers[5];
        shadowLoopers[0] =
            SkBlurDrawLooper::Create(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10),
                                     SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                     SkBlurDrawLooper::kOverrideColor_BlurFlag |
                                     SkBlurDrawLooper::kHighQuality_BlurFlag);
        SkAutoUnref aurL0(shadowLoopers[0]);
        shadowLoopers[1] =
            SkBlurDrawLooper::Create(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10),
                                     SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                     SkBlurDrawLooper::kOverrideColor_BlurFlag);
        SkAutoUnref aurL1(shadowLoopers[1]);
        shadowLoopers[2] =
            SkBlurDrawLooper::Create(SK_ColorBLACK,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                     SkIntToScalar(5),
                                     SkIntToScalar(10),
                                     SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                     SkBlurDrawLooper::kHighQuality_BlurFlag);
        SkAutoUnref aurL2(shadowLoopers[2]);
        shadowLoopers[3] =
            SkBlurDrawLooper::Create(0x7FFF0000,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                     SkIntToScalar(-5), SkIntToScalar(-10),
                                     SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                     SkBlurDrawLooper::kOverrideColor_BlurFlag |
                                     SkBlurDrawLooper::kHighQuality_BlurFlag);
        SkAutoUnref aurL3(shadowLoopers[3]);
        shadowLoopers[4] =
            SkBlurDrawLooper::Create(SK_ColorBLACK, SkIntToScalar(0),
                                     SkIntToScalar(5), SkIntToScalar(5),
                                     SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                     SkBlurDrawLooper::kOverrideColor_BlurFlag |
                                     SkBlurDrawLooper::kHighQuality_BlurFlag);
        SkAutoUnref aurL4(shadowLoopers[4]);

        static const struct {
            SkColor fColor;
            SkScalar fStrokeWidth;
        } gRec[] = {
            { SK_ColorRED,      -SK_Scalar1 },
            { SK_ColorGREEN,    SkIntToScalar(4) },
            { SK_ColorBLUE,     SkIntToScalar(0)},
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(shadowLoopers); ++i) {
            SkAutoCanvasRestore acr(canvas, true);

            paint.setLooper(shadowLoopers[i]);

            canvas->translate(SkIntToScalar((unsigned int)i*40), SkIntToScalar(0));
            setup(&paint, gRec[0].fColor, gRec[0].fStrokeWidth);
            canvas->drawRect(fRect, paint);

            canvas->translate(SkIntToScalar(0), SkIntToScalar(40));
            setup(&paint, gRec[1].fColor, gRec[1].fStrokeWidth);
            canvas->drawPath(fCirclePath, paint);

            canvas->translate(SkIntToScalar(0), SkIntToScalar(40));
            setup(&paint, gRec[2].fColor, gRec[2].fStrokeWidth);
            canvas->drawPath(fCirclePath, paint);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShadowsGM; }
static GMRegistry reg(MyFactory);

}
