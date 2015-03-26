
/*
 * Copyright 2012 Intel Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBlurDrawLooper.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkGradientShader.h"
#include "SkMatrix.h"
#include "SkRandom.h"
#include "SkTArray.h"

namespace skiagm {

class CircleGM : public GM {
public:
    CircleGM() {
        this->setBGColor(0xFF000000);
        this->makePaints();
        this->makeMatrices();
    }

protected:

    SkString onShortName() override {
        return SkString("circles");
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
        // AA with mask filter
        SkPaint p;
        p.setAntiAlias(true);
        SkMaskFilter* mf = SkBlurMaskFilter::Create(
                               kNormal_SkBlurStyle,
                               SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5)),
                               SkBlurMaskFilter::kHighQuality_BlurFlag);
        p.setMaskFilter(mf)->unref();
        fPaints.push_back(p);
        }

        {
        // AA with radial shader
        SkPaint p;
        p.setAntiAlias(true);
        SkPoint center = SkPoint::Make(SkIntToScalar(40), SkIntToScalar(40));
        SkColor colors[] = { SK_ColorBLUE, SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[] = { 0, SK_ScalarHalf, SK_Scalar1 };
        SkShader* s = SkGradientShader::CreateRadial(center,
                                                     SkIntToScalar(20),
                                                     colors,
                                                     pos,
                                                     SK_ARRAY_COUNT(colors),
                                                     SkShader::kClamp_TileMode);
        p.setShader(s)->unref();
        fPaints.push_back(p);
        }

        {
        // AA with blur
        SkPaint p;
        p.setAntiAlias(true);
        SkBlurDrawLooper* shadowLooper =
            SkBlurDrawLooper::Create(SK_ColorBLUE,
                                     SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(10)),
                                     SkIntToScalar(5), SkIntToScalar(10),
                                     SkBlurDrawLooper::kIgnoreTransform_BlurFlag |
                                     SkBlurDrawLooper::kOverrideColor_BlurFlag |
                                     SkBlurDrawLooper::kHighQuality_BlurFlag);
        SkAutoUnref aurL0(shadowLooper);
        p.setLooper(shadowLooper);
        fPaints.push_back(p);
        }

        {
        // AA with stroke style
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SkIntToScalar(3));
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
        p.setStrokeWidth(SkIntToScalar(2));
        fPaints.push_back(p);
        }
    }

    void makeMatrices() {
        {
        SkMatrix m;
        m.setScale(SkIntToScalar(2), SkIntToScalar(3));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setScale(SkIntToScalar(2), SkIntToScalar(2));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setSkew(SkIntToScalar(2), SkIntToScalar(3));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setSkew(SkIntToScalar(2), SkIntToScalar(2));
        fMatrices.push_back(m);
        }

        {
        SkMatrix m;
        m.setRotate(SkIntToScalar(30));
        fMatrices.push_back(m);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // Draw a giant AA circle as the background.
        SkISize size = this->getISize();
        SkScalar giantRadius = SkTMin(SkIntToScalar(size.fWidth),
                                      SkIntToScalar(size.fHeight)) / 2.f;
        SkPoint giantCenter = SkPoint::Make(SkIntToScalar(size.fWidth/2),
                                            SkIntToScalar(size.fHeight/2));
        SkPaint giantPaint;
        giantPaint.setAntiAlias(true);
        giantPaint.setColor(0x80808080);
        canvas->drawCircle(giantCenter.fX, giantCenter.fY, giantRadius, giantPaint);
        
        SkRandom rand;
        canvas->translate(20 * SK_Scalar1, 20 * SK_Scalar1);
        int i;
        for (i = 0; i < fPaints.count(); ++i) {
            canvas->save();
            // position the path, and make it at off-integer coords.
            canvas->translate(SK_Scalar1 * 200 * (i % 5) + SK_Scalar1 / 4,
                              SK_Scalar1 * 200 * (i / 5) + 3 * SK_Scalar1 / 4);
            SkColor color = rand.nextU();
            color |= 0xff000000;
            fPaints[i].setColor(color);

            canvas->drawCircle(SkIntToScalar(40), SkIntToScalar(40),
                               SkIntToScalar(20),
                               fPaints[i]);
            canvas->restore();
        }

        for (int j = 0; j < fMatrices.count(); ++j, ++i) {
            canvas->save();

            canvas->translate(SK_Scalar1 * 200 * (i % 5) + SK_Scalar1 / 4,
                              SK_Scalar1 * 200 * (i / 5) + 3 * SK_Scalar1 / 4);

            canvas->concat(fMatrices[j]);

            SkPaint paint;
            paint.setAntiAlias(true);

            SkColor color = rand.nextU();
            color |= 0xff000000;
            paint.setColor(color);

            canvas->drawCircle(SkIntToScalar(40), SkIntToScalar(40),
                               SkIntToScalar(20),
                               paint);

            canvas->restore();
        }
    }

private:
    typedef GM INHERITED;
    SkTArray<SkPaint> fPaints;
    SkTArray<SkMatrix> fMatrices;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new CircleGM; }
static GMRegistry reg(MyFactory);

}
