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
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"

using namespace skia_private;

namespace skiagm {

class OvalGM : public GM {
public:
    OvalGM() {
        this->setBGColor(0xFF000000);
        this->makePaints();
        this->makeMatrices();
    }

protected:
    SkString getName() const override { return SkString("ovals"); }

    SkISize getISize() override { return SkISize::Make(1200, 900); }

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

    SkColor genColor(SkRandom* rand) {
        SkScalar hsv[3];
        hsv[0] = rand->nextRangeF(0.0f, 360.0f);
        hsv[1] = rand->nextRangeF(0.75f, 1.0f);
        hsv[2] = rand->nextRangeF(0.75f, 1.0f);

        return ToolUtils::color_to_565(SkHSVToColor(hsv));
    }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(1);
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);
        const SkRect kOval = SkRect::MakeLTRB(-20, -30, 20, 30);

        const SkScalar kXStart = 60.0f;
        const SkScalar kYStart = 80.0f;
        const int kXStep = 150;
        const int kYStep = 160;
        int maxX = fMatrices.size();

        SkPaint rectPaint;
        rectPaint.setAntiAlias(true);
        rectPaint.setStyle(SkPaint::kStroke_Style);
        rectPaint.setStrokeWidth(SkIntToScalar(0));
        rectPaint.setColor(SK_ColorLTGRAY);

        int testCount = 0;
        for (int i = 0; i < fPaints.size(); ++i) {
            for (int j = 0; j < fMatrices.size(); ++j) {
                canvas->save();
                SkMatrix mat = fMatrices[j];
                // position the oval, and make it at off-integer coords.
                mat.postTranslate(kXStart + SK_Scalar1 * kXStep * (testCount % maxX) +
                                  SK_Scalar1 / 4,
                                  kYStart + SK_Scalar1 * kYStep * (testCount / maxX) +
                                  3 * SK_Scalar1 / 4);
                canvas->concat(mat);

                SkColor color = genColor(&rand);
                fPaints[i].setColor(color);

                canvas->drawRect(kOval, rectPaint);
                canvas->drawOval(kOval, fPaints[i]);

                canvas->restore();

                ++testCount;
            }
        }

        // special cases

        // non-scaled tall and skinny oval
        for (int i = 0; i < fPaints.size(); ++i) {
            SkRect oval = SkRect::MakeLTRB(-20, -60, 20, 60);
            canvas->save();
            // position the oval, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 2.55f + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);

            canvas->drawRect(oval, rectPaint);
            canvas->drawOval(oval, fPaints[i]);
            canvas->restore();
        }

        // non-scaled wide and short oval
        for (int i = 0; i < fPaints.size(); ++i) {
            SkRect oval = SkRect::MakeLTRB(-80, -30, 80, 30);
            canvas->save();
            // position the oval, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 4 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);

            canvas->drawRect(oval, rectPaint);
            canvas->drawOval(oval, fPaints[i]);
            canvas->restore();
        }

        // super skinny oval
        for (int i = 0; i < fPaints.size(); ++i) {
            SkRect oval = SkRect::MakeLTRB(0, -60, 1, 60);
            canvas->save();
            // position the oval, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 3.25f + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);

            canvas->drawOval(oval, fPaints[i]);
            canvas->restore();
        }

        // super short oval
        for (int i = 0; i < fPaints.size(); ++i) {
            SkRect oval = SkRect::MakeLTRB(-80, -1, 80, 0);
            canvas->save();
            // position the oval, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 2.5f + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);

            canvas->drawOval(oval, fPaints[i]);
            canvas->restore();
        }

        // radial gradient
        SkPoint center = SkPoint::Make(SkIntToScalar(0), SkIntToScalar(0));
        SkColor colors[] = { SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
        auto shader = SkGradientShader::MakeRadial(center, 20, colors, pos, std::size(colors),
                                                   SkTileMode::kClamp);

        for (int i = 0; i < fPaints.size(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 0 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);
            fPaints[i].setShader(shader);

            canvas->drawRect(kOval, rectPaint);
            canvas->drawOval(kOval, fPaints[i]);

            fPaints[i].setShader(nullptr);

            canvas->restore();
        }

        // reflected oval
        for (int i = 0; i < fPaints.size(); ++i) {
            SkRect oval = SkRect::MakeLTRB(-30, -30, 30, 30);
            canvas->save();
            // position the oval, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 5 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);
            canvas->rotate(90);
            canvas->scale(1, -1);
            canvas->scale(1, 0.66f);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);

            canvas->drawRect(oval, rectPaint);
            canvas->drawOval(oval, fPaints[i]);
            canvas->restore();
        }
    }

private:
    TArray<SkPaint> fPaints;
    TArray<SkMatrix> fMatrices;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new OvalGM; )

}  // namespace skiagm

DEF_SIMPLE_GM(open_ovals, canvas, 225, 110) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    const SkRect rect = SkRect::MakeWH(50, 100);
    const SkPathDirection dir = SkPathDirection::kCW;
    const int start = 1;

    // We stroke several open ovals to see how they behave
    canvas->translate(5, 5);

    auto doRow = [&](const SkPath& p) {
        canvas->drawPath(p, paint);
        canvas->translate(55, 0);
    };

    // Default case (left open) looks like an oval
    SkPath path;
    path.addOpenOval(rect, dir, start);
    doRow(path);

    // Closing makes us technically be an oval, but should look the same
    path.close();
    doRow(path);

    // Moving before the oval adds a line to the start
    path.reset();
    path.moveTo(rect.center());
    path.addOpenOval(rect, dir, start);
    doRow(path);

    // Similarly, lineTo after the oval starts from the start/end point
    path.reset();
    path.addOpenOval(rect, dir, start);
    path.lineTo(rect.center());
    doRow(path);
}
