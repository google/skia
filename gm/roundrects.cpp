/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

namespace skiagm {

static SkColor gen_color(SkRandom* rand) {
    SkScalar hsv[3];
    hsv[0] = rand->nextRangeF(0.0f, 360.0f);
    hsv[1] = rand->nextRangeF(0.75f, 1.0f);
    hsv[2] = rand->nextRangeF(0.75f, 1.0f);

    return ToolUtils::color_to_565(SkHSVToColor(hsv));
}

class RoundRectGM : public GM {
public:
    RoundRectGM() {
        this->setBGColor(0xFF000000);
        this->makePaints();
        this->makeMatrices();
    }

protected:

    SkString onShortName() override {
        return SkString("roundrects");
    }

    SkISize onISize() override {
        return SkISize::Make(1200, 900);
    }

    void makePaints() {
        {
            // no AA
            SkPaint p;
            fPaints.push_back(p);
        }

        {
            // AA
            SkPaint p;
            p.setAntiAlias(true);
            fPaints.push_back(p);
        }

        {
            // AA with stroke style
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(SkIntToScalar(5));
            fPaints.push_back(p);
        }

        {
            // AA with stroke style, width = 0
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            fPaints.push_back(p);
        }

        {
            // AA with stroke and fill style
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStrokeAndFill_Style);
            p.setStrokeWidth(SkIntToScalar(3));
            fPaints.push_back(p);
        }
    }

    void makeMatrices() {
        {
            SkMatrix m;
            m.setIdentity();
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setScale(SkIntToScalar(3), SkIntToScalar(2));
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setScale(SkIntToScalar(2), SkIntToScalar(2));
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setScale(SkIntToScalar(1), SkIntToScalar(2));
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setScale(SkIntToScalar(4), SkIntToScalar(1));
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setRotate(SkIntToScalar(90));
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setSkew(SkIntToScalar(2), SkIntToScalar(3));
            fMatrices.push_back(m);
        }

        {
            SkMatrix m;
            m.setRotate(SkIntToScalar(60));
            fMatrices.push_back(m);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(1);
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);
        const SkRect kRect = SkRect::MakeLTRB(-20, -30, 20, 30);
        SkRRect circleRRect;
        circleRRect.setRectXY(kRect, 5, 5);

        const SkScalar kXStart = 60.0f;
        const SkScalar kYStart = 80.0f;
        const int kXStep = 150;
        const int kYStep = 160;
        int maxX = fMatrices.count();

        SkPaint rectPaint;
        rectPaint.setAntiAlias(true);
        rectPaint.setStyle(SkPaint::kStroke_Style);
        rectPaint.setStrokeWidth(SkIntToScalar(0));
        rectPaint.setColor(SK_ColorLTGRAY);

        int testCount = 0;
        for (int i = 0; i < fPaints.count(); ++i) {
            for (int j = 0; j < fMatrices.count(); ++j) {
                canvas->save();
                SkMatrix mat = fMatrices[j];
                // position the roundrect, and make it at off-integer coords.
                mat.postTranslate(kXStart + SK_Scalar1 * kXStep * (testCount % maxX) +
                                  SK_Scalar1 / 4,
                                  kYStart + SK_Scalar1 * kYStep * (testCount / maxX) +
                                  3 * SK_Scalar1 / 4);
                canvas->concat(mat);

                SkColor color = gen_color(&rand);
                fPaints[i].setColor(color);

                canvas->drawRect(kRect, rectPaint);
                canvas->drawRRect(circleRRect, fPaints[i]);

                canvas->restore();

                ++testCount;
            }
        }

        // special cases

        // non-scaled tall and skinny roundrect
        for (int i = 0; i < fPaints.count(); ++i) {
            SkRect rect = SkRect::MakeLTRB(-20, -60, 20, 60);
            SkRRect ellipseRect;
            ellipseRect.setRectXY(rect, 5, 10);

            canvas->save();
            // position the roundrect, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 2.55f + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4);

            SkColor color = gen_color(&rand);
            fPaints[i].setColor(color);

            canvas->drawRect(rect, rectPaint);
            canvas->drawRRect(ellipseRect, fPaints[i]);
            canvas->restore();
        }

        // non-scaled wide and short roundrect
        for (int i = 0; i < fPaints.count(); ++i) {
            SkRect rect = SkRect::MakeLTRB(-80, -30, 80, 30);
            SkRRect ellipseRect;
            ellipseRect.setRectXY(rect, 20, 5);

            canvas->save();
            // position the roundrect, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 4 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = gen_color(&rand);
            fPaints[i].setColor(color);

            canvas->drawRect(rect, rectPaint);
            canvas->drawRRect(ellipseRect, fPaints[i]);
            canvas->restore();
        }

        // super skinny roundrect
        for (int i = 0; i < fPaints.count(); ++i) {
            SkRect rect = SkRect::MakeLTRB(0, -60, 1, 60);
            SkRRect circleRect;
            circleRect.setRectXY(rect, 5, 5);

            canvas->save();
            // position the roundrect, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 3.25f + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4);

            SkColor color = gen_color(&rand);
            fPaints[i].setColor(color);

            canvas->drawRRect(circleRect, fPaints[i]);
            canvas->restore();
        }

        // super short roundrect
        for (int i = 0; i < fPaints.count(); ++i) {
            SkRect rect = SkRect::MakeLTRB(-80, -1, 80, 0);
            SkRRect circleRect;
            circleRect.setRectXY(rect, 5, 5);

            canvas->save();
            // position the roundrect, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 2.5f + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = gen_color(&rand);
            fPaints[i].setColor(color);

            canvas->drawRRect(circleRect, fPaints[i]);
            canvas->restore();
        }

        // radial gradient
        SkPoint center = SkPoint::Make(SkIntToScalar(0), SkIntToScalar(0));
        SkColor colors[] = { SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
        auto shader = SkGradientShader::MakeRadial(center, 20, colors, pos, SK_ARRAY_COUNT(colors),
                                                   SkTileMode::kClamp);

        for (int i = 0; i < fPaints.count(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 0 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = gen_color(&rand);
            fPaints[i].setColor(color);
            fPaints[i].setShader(shader);

            canvas->drawRect(kRect, rectPaint);
            canvas->drawRRect(circleRRect, fPaints[i]);

            fPaints[i].setShader(nullptr);

            canvas->restore();
        }

        // strokes and radii
        {
            SkScalar radii[][2] = {
                {10,10},
                {5,15},
                {5,15},
                {5,15}
            };

            SkScalar strokeWidths[] = {
                20, 10, 20, 40
            };

            for (int i = 0; i < 4; ++i) {
                SkRRect circleRect;
                circleRect.setRectXY(kRect, radii[i][0], radii[i][1]);

                canvas->save();
                // position the roundrect, and make it at off-integer coords.
                canvas->translate(kXStart + SK_Scalar1 * kXStep * 5 + SK_Scalar1 / 4,
                                  kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                                  SK_ScalarHalf * kYStep);

                SkColor color = gen_color(&rand);

                SkPaint p;
                p.setAntiAlias(true);
                p.setStyle(SkPaint::kStroke_Style);
                p.setStrokeWidth(strokeWidths[i]);
                p.setColor(color);

                canvas->drawRRect(circleRect, p);
                canvas->restore();
            }
        }

        // test old entry point ( https://bug.skia.org/3786 )
        {
            canvas->save();

            canvas->translate(kXStart + SK_Scalar1 * kXStep * 5 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * 4 + SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            const SkColor color = gen_color(&rand);

            SkPaint p;
            p.setColor(color);

            const SkRect oooRect = { 20, 30, -20, -30 };     // intentionally out of order
            canvas->drawRoundRect(oooRect, 10, 10, p);

            canvas->restore();
        }

        // rrect with stroke > radius/2
        {
            SkRect smallRect = { -30, -20, 30, 20 };
            SkRRect circleRect;
            circleRect.setRectXY(smallRect, 5, 5);

            canvas->save();
            // position the roundrect, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 5 + SK_Scalar1 / 4,
                              kYStart - SK_Scalar1 * kYStep + 73 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = gen_color(&rand);

            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(25);
            p.setColor(color);

            canvas->drawRRect(circleRect, p);
            canvas->restore();
        }
    }

private:
    SkTArray<SkPaint> fPaints;
    SkTArray<SkMatrix> fMatrices;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new RoundRectGM; )

}  // namespace skiagm
