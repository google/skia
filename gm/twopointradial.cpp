/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkGradientShader.h"

static void intToScalars(SkScalar dst[], const int src[], int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = SkIntToScalar(src[i]);
    }
}

static void drawGrad(SkCanvas* canvas, const SkScalar d0[], const SkScalar d1[]) {
    const SkRect bounds = SkRect::MakeXYWH(SkIntToScalar(-50),
                                           SkIntToScalar(-50),
                                           SkIntToScalar(200),
                                           SkIntToScalar(100));

    SkPoint c0 = { d0[0], d0[1] };
    SkScalar r0 = d0[2];
    SkPoint c1 = { d1[0], d1[1] };
    SkScalar r1 = d1[2];

    SkColor colors[] = { SK_ColorGREEN, SK_ColorRED };
    SkPaint paint;
    paint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&paint);

    SkString str;
    str.printf("%g,%g,%g  %g,%g,%g",
               SkScalarToFloat(c0.fX), SkScalarToFloat(c0.fY), SkScalarToFloat(r0),
               SkScalarToFloat(c1.fX), SkScalarToFloat(c1.fY), SkScalarToFloat(r1));
    canvas->drawText(str.c_str(), str.size(),
                     bounds.fLeft, bounds.fTop - paint.getTextSize()/2, paint);

    paint.setShader(SkGradientShader::CreateTwoPointConical(c0, r0, c1, r1,
                                                            colors, NULL, 2,
                                                            SkShader::kClamp_TileMode))->unref();
    canvas->drawRect(bounds, paint);

    paint.setShader(NULL);
    paint.setColor(0x66000000);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawCircle(c0.fX, c0.fY, r0, paint);
    canvas->drawCircle(c1.fX, c1.fY, r1, paint);
    canvas->drawRect(bounds, paint);
}

class TwoPointRadialGM : public skiagm::GM {
public:
    TwoPointRadialGM() {}

protected:
    SkString onShortName() {
        return SkString("twopointconical");
    }

    SkISize onISize() { return SkISize::Make(480, 780); }

    virtual void onDraw(SkCanvas* canvas) {
        if (false) {
            SkPaint paint;
            paint.setColor(SK_ColorBLUE);
            canvas->drawRect(
                    SkRect::MakeWH(SkIntToScalar(this->getISize().fWidth),
                                   SkIntToScalar(this->getISize().fHeight)),
                    paint);
        }
        SkPaint paint;
        const int R0 = 20;
        const int R1 = 40;

        const SkScalar DX = SkIntToScalar(250);
        const SkScalar DY = SkIntToScalar(130);

        canvas->translate(SkIntToScalar(60), SkIntToScalar(70));

        static const int gData[] = {
            0, 0, R0,       0, 0, R1,
            0, 0, R0,       20, 0, R1,
            0, 0, R0,       25, 0, R1,
            0, 0, R0,       100, 0, R1,
            0, 0, R0,       25, 0, R0,
            0, 0, R0,       100, 0, R0,
        };

        int count = SK_ARRAY_COUNT(gData) / 6;
        for (int i = 0; i < count; ++i) {
            SkScalar data[6];
            intToScalars(data, &gData[i * 6], 6);

            int n = canvas->save();
            drawGrad(canvas, &data[0], &data[3]);
            canvas->translate(DX, 0);
            drawGrad(canvas, &data[3], &data[0]);
            canvas->restoreToCount(n);
            canvas->translate(0, DY);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* F(void*) { return new TwoPointRadialGM; }

static skiagm::GMRegistry gR(F);
