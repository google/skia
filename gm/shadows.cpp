/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlurDrawLooper.h"
#include "src/core/SkBlurMask.h"

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER

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
    SkBitmap fBitmap;

protected:
    void onOnceBeforeDraw() override {
        this->setBGColor(0xFFDDDDDD);
        fCirclePath.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(10) );
        fRect.set(SkIntToScalar(10), SkIntToScalar(10),
                  SkIntToScalar(30), SkIntToScalar(30));
        fBitmap.allocPixels(SkImageInfo::Make(20, 20, SkColorType::kAlpha_8_SkColorType,
                            kPremul_SkAlphaType));
        SkCanvas canvas(fBitmap);
        canvas.clear(0x0);
        SkPaint p;
        canvas.drawRect(SkRect::MakeXYWH(10, 0, 10, 10), p);
        canvas.drawRect(SkRect::MakeXYWH(0, 10, 10, 10), p);
    }

    SkString onShortName() override {
        return SkString("shadows");
    }

    SkISize onISize() override {
        return SkISize::Make(200, 200);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkDrawLooper> shadowLoopers[] = {
              SkBlurDrawLooper::Make(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10)),
              SkBlurDrawLooper::Make(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10)),
              SkBlurDrawLooper::Make(SK_ColorBLACK,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                     SkIntToScalar(5),
                                     SkIntToScalar(10)),
              SkBlurDrawLooper::Make(0x7FFF0000,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                     SkIntToScalar(-5), SkIntToScalar(-10)),
            SkBlurDrawLooper::Make(SK_ColorBLACK, SkIntToScalar(0),
                                     SkIntToScalar(5), SkIntToScalar(5)),
        };

        constexpr struct {
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

            // see bug.skia.org/562 (reference, draws correct)
            canvas->translate(0, 40);
            paint.setColor(SK_ColorBLACK);
            canvas->drawBitmap(fBitmap, 10, 10, &paint);

            canvas->translate(0, 40);
            paint.setShader(fBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));

            // see bug.skia.org/562 (shows bug as reported)
            paint.setStyle(SkPaint::kFill_Style);
            canvas->drawRect(SkRect::MakeXYWH(10, 10, 20, 20), paint);
            paint.setShader(nullptr);
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ShadowsGM; )

}

#endif
