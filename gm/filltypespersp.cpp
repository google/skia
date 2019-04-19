/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGradientShader.h"
#include "SkPath.h"

namespace skiagm {

class FillTypePerspGM : public GM {
    SkPath fPath;
public:
    FillTypePerspGM() {}

    void makePath() {
        if (fPath.isEmpty()) {
            const SkScalar radius = SkIntToScalar(45);
            fPath.addCircle(SkIntToScalar(50), SkIntToScalar(50), radius);
            fPath.addCircle(SkIntToScalar(100), SkIntToScalar(100), radius);
        }
    }

protected:

    SkString onShortName() override {
        return SkString("filltypespersp");
    }

    SkISize onISize() override {
        return SkISize::Make(835, 840);
    }

    void showPath(SkCanvas* canvas, int x, int y, SkPath::FillType ft,
                  SkScalar scale, const SkPaint& paint) {
        const SkRect r = { 0, 0, SkIntToScalar(150), SkIntToScalar(150) };

        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(r);
        canvas->drawColor(SK_ColorWHITE);
        fPath.setFillType(ft);
        canvas->translate(r.centerX(), r.centerY());
        canvas->scale(scale, scale);
        canvas->translate(-r.centerX(), -r.centerY());
        canvas->drawPath(fPath, paint);
        canvas->restore();
    }

    void showFour(SkCanvas* canvas, SkScalar scale, bool aa) {
        SkPaint paint;
        SkPoint center = SkPoint::Make(SkIntToScalar(100), SkIntToScalar(100));
        SkColor colors[] = {SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN};
        SkScalar pos[] = {0, SK_ScalarHalf, SK_Scalar1};
        paint.setShader(SkGradientShader::MakeRadial(center,
                                                     SkIntToScalar(100),
                                                     colors,
                                                     pos,
                                                     SK_ARRAY_COUNT(colors),
                                                     SkTileMode::kClamp));
        paint.setAntiAlias(aa);

        showPath(canvas,   0,   0, SkPath::kWinding_FillType,
                 scale, paint);
        showPath(canvas, 200,   0, SkPath::kEvenOdd_FillType,
                 scale, paint);
        showPath(canvas,  00, 200, SkPath::kInverseWinding_FillType,
                 scale, paint);
        showPath(canvas, 200, 200, SkPath::kInverseEvenOdd_FillType,
                 scale, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        this->makePath();

        // do perspective drawPaint as the background;
        SkPaint bkgnrd;
        SkPoint center = SkPoint::Make(SkIntToScalar(100),
                                       SkIntToScalar(100));
        SkColor colors[] = {SK_ColorBLACK, SK_ColorCYAN,
                            SK_ColorYELLOW, SK_ColorWHITE};
        SkScalar pos[] = {0, SK_ScalarHalf / 2,
                          3 * SK_ScalarHalf / 2, SK_Scalar1};
        bkgnrd.setShader(SkGradientShader::MakeRadial(center,
                                                      SkIntToScalar(1000),
                                                      colors,
                                                      pos,
                                                      SK_ARRAY_COUNT(colors),
                                                      SkTileMode::kClamp));
        canvas->save();
            canvas->translate(SkIntToScalar(100), SkIntToScalar(100));
            SkMatrix mat;
            mat.reset();
            mat.setPerspY(SK_Scalar1 / 1000);
            canvas->concat(mat);
            canvas->drawPaint(bkgnrd);
        canvas->restore();

        // draw the paths in perspective
        SkMatrix persp;
        persp.reset();
        persp.setPerspX(-SK_Scalar1 / 1800);
        persp.setPerspY(SK_Scalar1 / 500);
        canvas->concat(persp);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        const SkScalar scale = SkIntToScalar(5)/4;

        showFour(canvas, SK_Scalar1, false);
        canvas->translate(SkIntToScalar(450), 0);
        showFour(canvas, scale, false);

        canvas->translate(SkIntToScalar(-450), SkIntToScalar(450));
        showFour(canvas, SK_Scalar1, true);
        canvas->translate(SkIntToScalar(450), 0);
        showFour(canvas, scale, true);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new FillTypePerspGM; )

}
