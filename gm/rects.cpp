/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkMatrix.h"
#include "SkTArray.h"

namespace skiagm {

class RectsGM : public GM {
public:
    RectsGM() {
        this->setBGColor(0xFF000000);
        this->makePaints();
        this->makeMatrices();
        this->makeRects();
    }

protected:

    SkString onShortName() override {
        return SkString("rects");
    }

    SkISize onISize() override {
        return SkISize::Make(1200, 900);
    }

    void makePaints() {
        {
            // no AA
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            fPaints.push_back(p);
        }

        {
            // AA
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            fPaints.push_back(p);
        }

        {
            // AA with translucent
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setAlpha(0x66);
            fPaints.push_back(p);
        }

        {
            // AA with mask filter
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setMaskFilter(SkBlurMaskFilter::Make(
                                   kNormal_SkBlurStyle,
                                   SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                                   SkBlurMaskFilter::kHighQuality_BlurFlag));
            fPaints.push_back(p);
        }

        {
            // AA with radial shader
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            SkPoint center = SkPoint::Make(SkIntToScalar(-5), SkIntToScalar(30));
            SkColor colors[] = { SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN };
            SkScalar pos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
            p.setShader(SkGradientShader::MakeRadial(center, 20, colors, pos,
                                                     SK_ARRAY_COUNT(colors),
                                                     SkShader::kClamp_TileMode));
            fPaints.push_back(p);
        }

        {
            // AA with blur
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setLooper(SkBlurDrawLooper::Make(SK_ColorWHITE,
                                         SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                         SkIntToScalar(5), SkIntToScalar(10)));
            fPaints.push_back(p);
        }

        {
            // AA with stroke style
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(SkIntToScalar(3));
            fPaints.push_back(p);
        }

        {
            // AA with bevel-stroke style
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeJoin(SkPaint::kBevel_Join);
            p.setStrokeWidth(SkIntToScalar(3));
            fPaints.push_back(p);
        }

        {
            // AA with round-stroke style
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeJoin(SkPaint::kRound_Join);
            p.setStrokeWidth(SkIntToScalar(3));
            fPaints.push_back(p);
        }

        {
            // AA with stroke style, width = 0
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            fPaints.push_back(p);
        }

        {
            // AA with stroke style, width wider than rect width and/or height
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(SkIntToScalar(40));
            fPaints.push_back(p);
        }

        {
            // AA with stroke and fill style
            SkPaint p;
            p.setColor(SK_ColorWHITE);
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStrokeAndFill_Style);
            p.setStrokeWidth(SkIntToScalar(2));
            fPaints.push_back(p);
        }
    }

    void makeMatrices() {
        {
            // 1x1.5 scale
            SkMatrix m;
            m.setScale(1, 1.5f);
            fMatrices.push_back(m);
        }

        {
            // 1.5x1.5 scale
            SkMatrix m;
            m.setScale(1.5f, 1.5f);
            fMatrices.push_back(m);
        }

        {
            // 1x1.5 skew
            SkMatrix m;
            m.setSkew(1, 1.5f);
            fMatrices.push_back(m);
        }

        {
            // 1.5x1.5 skew
            SkMatrix m;
            m.setSkew(1.5f, 1.5f);
            fMatrices.push_back(m);
        }

        {
            // 30 degree rotation
            SkMatrix m;
            m.setRotate(SkIntToScalar(30));
            fMatrices.push_back(m);
        }

        {
            // 90 degree rotation
            SkMatrix m;
            m.setRotate(SkIntToScalar(90));
            fMatrices.push_back(m);
        }
    }

    void makeRects() {
        {
            // small square
            SkRect r = SkRect::MakeLTRB(0, 0, 30, 30);
            fRects.push_back(r);
        }

        {
            // thin vertical
            SkRect r = SkRect::MakeLTRB(0, 0, 2, 40);
            fRects.push_back(r);
        }

        {
            // thin horizontal
            SkRect r = SkRect::MakeLTRB(0, 0, 40, 2);
            fRects.push_back(r);
        }

        {
            // very thin
            SkRect r = SkRect::MakeLTRB(0, 0, 0.25f, 10);
            fRects.push_back(r);
        }

        {
            // zaftig
            SkRect r = SkRect::MakeLTRB(0, 0, 60, 60);
            fRects.push_back(r);
        }
    }

    // position the current test on the canvas
    static void position(SkCanvas* canvas, int testCount) {
        canvas->translate(SK_Scalar1 * 100 * (testCount % 10) + SK_Scalar1 / 4,
                          SK_Scalar1 * 100 * (testCount / 10) + 3 * SK_Scalar1 / 4);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);

        int testCount = 0;

        for (int i = 0; i < fPaints.count(); ++i) {
            for (int j = 0; j < fRects.count(); ++j, ++testCount) {
                canvas->save();
                this->position(canvas, testCount);
                canvas->drawRect(fRects[j], fPaints[i]);
                canvas->restore();
            }
        }

        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setAntiAlias(true);

        for (int i = 0; i < fMatrices.count(); ++i) {
            for (int j = 0; j < fRects.count(); ++j, ++testCount) {
                canvas->save();
                this->position(canvas, testCount);
                canvas->concat(fMatrices[i]);
                canvas->drawRect(fRects[j], paint);
                canvas->restore();
            }
        }
    }

private:
    SkTArray<SkPaint>  fPaints;
    SkTArray<SkMatrix> fMatrices;
    SkTArray<SkRect>   fRects;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new RectsGM; }
static GMRegistry reg(MyFactory);

}
