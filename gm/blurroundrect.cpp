/*
* Copyright 2013 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkBlurMask.h"

/*
 * Spits out an arbitrary gradient to test blur with shader on paint
 */
static sk_sp<SkShader> MakeRadial() {
    SkPoint pts[2] = {
        { 0, 0 },
        { SkIntToScalar(100), SkIntToScalar(100) }
    };
    SkTileMode tm = SkTileMode::kClamp;
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, };
    const SkScalar pos[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
    SkMatrix scale;
    scale.setScale(0.5f, 0.5f);
    scale.postTranslate(5.f, 5.f);
    SkPoint center0, center1;
    center0.set(sk_float_midpoint(pts[0].fX, pts[1].fX),
                sk_float_midpoint(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 colors, pos, std::size(colors), tm,
                                                 0, &scale);
}

// Simpler blurred RR test cases where all the radii are the same.
class SimpleBlurRoundRectGM : public skiagm::GM {
    SkString getName() const override { return SkString("simpleblurroundrect"); }

    SkISize getISize() override { return {1000, 500}; }

    bool runAsBench() const override { return true; }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(1.5f, 1.5f);
        canvas->translate(50,50);

        const float blurRadii[] = {1.f, 5.f, 10.f, 20.f};
        const float cornerRadii[] = {1.f, 5.f, 10.f, 20.f};
        const SkRect r = SkRect::MakeWH(25.f, 25.f);
        for (size_t row = 0; row < std::size(blurRadii); ++row) {
            SkAutoCanvasRestore autoRestore(canvas, true);
            canvas->translate(0, (r.height() + 50.f) * row);
            for (size_t pair = 0; pair < std::size(cornerRadii); ++pair) {
                SkPaint paint;
                paint.setColor(SK_ColorBLACK);
                paint.setMaskFilter(SkMaskFilter::MakeBlur(
                        kNormal_SkBlurStyle, SkBlurMask::ConvertRadiusToSigma(blurRadii[row])));
                SkRRect rrect;
                rrect.setRectXY(r, cornerRadii[pair], cornerRadii[pair]);

                // Even-indexed columns are without a gradient
                canvas->drawRRect(rrect, paint);
                canvas->translate(r.width() + 50.f, 0);

                // Odd-indexed columns have a gradient
                paint.setShader(MakeRadial());
                canvas->drawRRect(rrect, paint);
                canvas->translate(r.width() + 50.f, 0);
            }
        }
    }
};

DEF_GM(return new SimpleBlurRoundRectGM();)

// From crbug.com/1138810
DEF_SIMPLE_GM(blur_large_rrects, canvas, 300, 300) {
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 20.f));

    auto rect = SkRect::MakeLTRB(5.f, -20000.f, 240.f,  25.f);
    SkRRect rrect = SkRRect::MakeRectXY(rect, 40.f, 40.f);
    for (int i = 0; i < 4; ++i) {
        SkColor4f color{(i & 1) ? 1.f : 0.f,
                        (i & 2) ? 1.f : 0.f,
                        (i < 2) ? 1.f : 0.f,
                        1.f};
        paint.setColor(color);
        canvas->drawRRect(rrect, paint);
        canvas->rotate(90.f, 150.f, 150.f);
    }
}
