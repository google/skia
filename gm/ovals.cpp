/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkTArray.h"
#include "SkRandom.h"
#include "SkMatrix.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkBlurDrawLooper.h"
#include "SkRect.h"

namespace skiagm {

class OvalGM : public GM {
public:
    OvalGM() {
        this->setBGColor(0xFF000000);
        this->makePaints();
        this->makeMatrices();
    }

protected:

    SkString onShortName() override {
        return SkString("ovals");
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

    SkColor genColor(SkRandom* rand) {
        SkScalar hsv[3];
        hsv[0] = rand->nextRangeF(0.0f, 360.0f);
        hsv[1] = rand->nextRangeF(0.75f, 1.0f);
        hsv[2] = rand->nextRangeF(0.75f, 1.0f);

        return sk_tool_utils::color_to_565(SkHSVToColor(hsv));
    }

    void onDraw(SkCanvas* canvas) override {
        SkRandom rand(1);
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);
        SkRect oval = SkRect::MakeLTRB(-20, -30, 20, 30);

        const SkScalar kXStart = 60.0f;
        const SkScalar kYStart = 80.0f;
        const int kXStep = 150;
        const int kYStep = 160;
        int maxX = fMatrices.count();

        SkPaint rectPaint;
        rectPaint.setAntiAlias(true);
        rectPaint.setStyle(SkPaint::kStroke_Style);
        rectPaint.setStrokeWidth(SkIntToScalar(0));
        rectPaint.setColor(sk_tool_utils::color_to_565(SK_ColorLTGRAY));

        int testCount = 0;
        for (int i = 0; i < fPaints.count(); ++i) {
            for (int j = 0; j < fMatrices.count(); ++j) {
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

                canvas->drawRect(oval, rectPaint);
                canvas->drawOval(oval, fPaints[i]);

                canvas->restore();

                ++testCount;
            }
        }

        // special cases

        // non-scaled tall and skinny oval
        for (int i = 0; i < fPaints.count(); ++i) {
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
        for (int i = 0; i < fPaints.count(); ++i) {
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
        for (int i = 0; i < fPaints.count(); ++i) {
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
        for (int i = 0; i < fPaints.count(); ++i) {
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
        auto shader = SkGradientShader::MakeRadial(center, 20, colors, pos, SK_ARRAY_COUNT(colors),
                                                   SkShader::kClamp_TileMode);

        for (int i = 0; i < fPaints.count(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(kXStart + SK_Scalar1 * kXStep * 0 + SK_Scalar1 / 4,
                              kYStart + SK_Scalar1 * kYStep * i + 3 * SK_Scalar1 / 4 +
                              SK_ScalarHalf * kYStep);

            SkColor color = genColor(&rand);
            fPaints[i].setColor(color);
            fPaints[i].setShader(shader);

            canvas->drawRect(oval, rectPaint);
            canvas->drawOval(oval, fPaints[i]);

            fPaints[i].setShader(nullptr);

            canvas->restore();
        }

        // reflected oval
        for (int i = 0; i < fPaints.count(); ++i) {
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
    SkTArray<SkPaint> fPaints;
    SkTArray<SkMatrix> fMatrices;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new OvalGM; }
static GMRegistry reg(MyFactory);

}
