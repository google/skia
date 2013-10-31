/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkUnitMappers.h"

namespace skiagm {

static void makebm(SkBitmap* bm, SkBitmap::Config config, int w, int h) {
    bm->setConfig(config, w, h);
    bm->allocPixels();
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkScalar    s = SkIntToScalar(SkMin32(w, h));
    static const SkPoint     kPts0[] = { { 0, 0 }, { s, s } };
    static const SkPoint     kPts1[] = { { s/2, 0 }, { s/2, s } };
    static const SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    static const SkColor kColors0[] = {0x80F00080, 0xF0F08000, 0x800080F0 };
    static const SkColor kColors1[] = {0xF08000F0, 0x8080F000, 0xF000F080 };


    SkPaint     paint;

    SkUnitMapper*   um = NULL;

    um = new SkCosineMapper;

    SkAutoUnref au(um);

    paint.setShader(SkGradientShader::CreateLinear(kPts0, kColors0, kPos,
                    SK_ARRAY_COUNT(kColors0), SkShader::kClamp_TileMode, um))->unref();
    canvas.drawPaint(paint);
    paint.setShader(SkGradientShader::CreateLinear(kPts1, kColors1, kPos,
                    SK_ARRAY_COUNT(kColors1), SkShader::kClamp_TileMode))->unref();
    canvas.drawPaint(paint);
}

///////////////////////////////////////////////////////////////////////////////

struct LabeledMatrix {
    SkMatrix    fMatrix;
    const char* fLabel;
};

class ShaderText3GM : public GM {
public:
    ShaderText3GM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:

    SkString onShortName() {
        return SkString("shadertext3");
    }

    SkISize onISize() { return make_isize(800, 1000); }

    virtual void onDraw(SkCanvas* canvas) {
        static const char kText[] = "B";
        static const int kTextLen = SK_ARRAY_COUNT(kText) - 1;
        static const int kPointSize = 300;

        static SkBitmap bmp;
        if (bmp.isNull()) {
            makebm(&bmp, SkBitmap::kARGB_8888_Config, kPointSize / 4, kPointSize / 4);
        }

        SkPaint bmpPaint;
        bmpPaint.setAntiAlias(true);
        bmpPaint.setFilterLevel(SkPaint::kLow_FilterLevel);
        bmpPaint.setAlpha(0x80);
        canvas->drawBitmap(bmp, 5.f, 5.f, &bmpPaint);

        SkPaint outlinePaint;
        outlinePaint.setAntiAlias(true);
        outlinePaint.setTextSize(SkIntToScalar(kPointSize));
        outlinePaint.setStyle(SkPaint::kStroke_Style);
        outlinePaint.setStrokeWidth(0.f);

        canvas->translate(15.f, 15.f);

        // draw glyphs scaled up
        canvas->scale(2.f, 2.f);

        static const SkShader::TileMode kTileModes[] = {
            SkShader::kRepeat_TileMode,
            SkShader::kMirror_TileMode,
        };

        // position the baseline of the first run
        canvas->translate(0.f, 0.75f * kPointSize);

        canvas->save();
        int i = 0;
        for (size_t tm0 = 0; tm0 < SK_ARRAY_COUNT(kTileModes); ++tm0) {
            for (size_t tm1 = 0; tm1 < SK_ARRAY_COUNT(kTileModes); ++tm1) {
                SkAutoTUnref<SkShader> shader(SkShader::CreateBitmapShader(bmp,
                                                                           kTileModes[tm0],
                                                                           kTileModes[tm1]));
                SkMatrix localM;
                localM.setTranslate(5.f, 5.f);
                localM.postRotate(20);
                localM.postScale(1.15f, .85f);
                shader->setLocalMatrix(localM);

                SkPaint fillPaint;
                fillPaint.setAntiAlias(true);
                fillPaint.setTextSize(SkIntToScalar(kPointSize));
                fillPaint.setFilterLevel(SkPaint::kLow_FilterLevel);
                fillPaint.setShader(shader);

                canvas->drawText(kText, kTextLen, 0, 0, fillPaint);
                canvas->drawText(kText, kTextLen, 0, 0, outlinePaint);
                SkScalar w = fillPaint.measureText(kText, kTextLen);
                canvas->translate(w + 10.f, 0.f);
                ++i;
                if (!(i % 2)) {
                    canvas->restore();
                    canvas->translate(0, 0.75f * kPointSize);
                    canvas->save();
                }
            }
        }
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#ifndef SK_BUILD_FOR_ANDROID
static GM* MyFactory(void*) { return new ShaderText3GM; }
static GMRegistry reg(MyFactory);
#endif
}
