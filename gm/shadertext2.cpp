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
    static const SkPoint     kPts1[] = { { s, 0 }, { 0, s } };
    static const SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    static const SkColor kColors0[] = {0x40FF00FF, 0xF0FFFF00, 0x4000FFFF };
    static const SkColor kColors1[] = {0xF0FF00FF, 0x80FFFF00, 0xF000FFFF };


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

class ShaderText2GM : public GM {
public:
    ShaderText2GM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:

    SkString onShortName() {
        return SkString("shadertext2");
    }

    SkISize onISize() { return make_isize(1800, 900); }

    virtual void onDraw(SkCanvas* canvas) {
        static const char kText[] = "SKIA";
        static const int kTextLen = SK_ARRAY_COUNT(kText) - 1;
        static const int kPointSize = 55;

        SkTDArray<LabeledMatrix> matrices;
        matrices.append()->fMatrix.reset();
        matrices.top().fLabel = "Identity";
        matrices.append()->fMatrix.setScale(1.2f, 0.8f);
        matrices.top().fLabel = "Scale";
        matrices.append()->fMatrix.setRotate(10.f);
        matrices.top().fLabel = "Rotate";
        matrices.append()->fMatrix.reset();
        matrices.top().fMatrix.setPerspX(-0.0015f);
        matrices.top().fMatrix.setPerspY(+0.0015f);
        matrices.top().fLabel = "Persp";

        SkTDArray<LabeledMatrix> localMatrices;
        localMatrices.append()->fMatrix.reset();
        localMatrices.top().fLabel = "Identity";
        localMatrices.append()->fMatrix.setScale(2.5f, 0.2f);
        localMatrices.top().fLabel = "Scale";
        localMatrices.append()->fMatrix.setRotate(45.f);
        localMatrices.top().fLabel = "Rotate";
        localMatrices.append()->fMatrix.reset();
        localMatrices.top().fMatrix.setPerspX(-0.007f);
        localMatrices.top().fMatrix.setPerspY(+0.008f);
        localMatrices.top().fLabel = "Persp";

        static SkBitmap bmp;
        if (bmp.isNull()) {
            makebm(&bmp, SkBitmap::kARGB_8888_Config, kPointSize / 2, kPointSize / 2);
        }

        SkAutoTUnref<SkShader> shader(SkShader::CreateBitmapShader(bmp,
                                                                   SkShader::kMirror_TileMode,
                                                                   SkShader::kRepeat_TileMode));
        SkPaint fillPaint;
        fillPaint.setAntiAlias(true);
        fillPaint.setTextSize(SkIntToScalar(kPointSize));
        fillPaint.setFilterLevel(SkPaint::kLow_FilterLevel);
        fillPaint.setShader(shader);

        SkPaint outlinePaint;
        outlinePaint.setAntiAlias(true);
        outlinePaint.setTextSize(SkIntToScalar(kPointSize));
        outlinePaint.setStyle(SkPaint::kStroke_Style);
        outlinePaint.setStrokeWidth(0.f);

        SkScalar w = fillPaint.measureText(kText, kTextLen);
        static SkScalar kPadY = 0.5f * kPointSize;
        static SkScalar kPadX = 1.5f * kPointSize;

        SkPaint strokePaint(fillPaint);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeWidth(kPointSize * 0.1f);

        SkPaint labelPaint;
        labelPaint.setColor(0xff000000);
        labelPaint.setAntiAlias(true);
        labelPaint.setTextSize(12.f);

        canvas->translate(15.f, 15.f);
        canvas->drawBitmap(bmp, 0, 0);
        canvas->translate(0, bmp.height() + labelPaint.getTextSize() + 15.f);

        static const char kLabelLabel[] = "localM / canvasM";
        canvas->drawText(kLabelLabel, strlen(kLabelLabel), 0, 0, labelPaint);
        canvas->translate(0, 15.f);

        canvas->save();
        SkScalar maxLabelW = 0;
        canvas->translate(0, kPadY / 2 + kPointSize);
        for (int lm = 0; lm < localMatrices.count(); ++lm) {
            canvas->drawText(matrices[lm].fLabel, strlen(matrices[lm].fLabel),
                             0, labelPaint.getTextSize() - 1, labelPaint);
            SkScalar labelW = labelPaint.measureText(matrices[lm].fLabel,
                                                     strlen(matrices[lm].fLabel));
            maxLabelW = SkMaxScalar(maxLabelW, labelW);
            canvas->translate(0.f, 2 * kPointSize + 2.5f * kPadY);
        }
        canvas->restore();

        canvas->translate(maxLabelW + kPadX / 2.f, 0.f);

        for (int s = 0; s < 2; ++s) {
            SkPaint& paint = s ? strokePaint : fillPaint;

            SkScalar columnH;
            for (int m = 0; m < matrices.count(); ++m) {
                columnH = 0;
                canvas->save();
                canvas->drawText(matrices[m].fLabel, strlen(matrices[m].fLabel),
                                 0, labelPaint.getTextSize() - 1, labelPaint);
                canvas->translate(0, kPadY / 2 + kPointSize);
                columnH += kPadY / 2 + kPointSize;
                for (int lm = 0; lm < localMatrices.count(); ++lm) {
                    shader->setLocalMatrix(localMatrices[lm].fMatrix);

                    canvas->save();
                        canvas->concat(matrices[m].fMatrix);
                        canvas->drawText(kText, kTextLen, 0, 0, paint);
                        canvas->drawText(kText, kTextLen, 0, 0, outlinePaint);
                    canvas->restore();

                    SkPath path;
                    path.arcTo(SkRect::MakeXYWH(-0.1f * w, 0.f,
                                                1.2f * w, 2.f * kPointSize),
                                                225.f, 359.f,
                                                false);
                    path.close();

                    canvas->translate(0.f, kPointSize + kPadY);
                    columnH += kPointSize + kPadY;

                    canvas->save();
                        canvas->concat(matrices[m].fMatrix);
                        canvas->drawTextOnPath(kText, kTextLen, path, NULL, paint);
                        canvas->drawTextOnPath(kText, kTextLen, path, NULL, outlinePaint);
                    canvas->restore();
                    SkPaint stroke;
                    stroke.setStyle(SkPaint::kStroke_Style);
                    canvas->translate(0.f, kPointSize + kPadY);
                    columnH += kPointSize + kPadY;
                }
                canvas->restore();
                canvas->translate(w + kPadX, 0.f);
            }
            if (0 == s) {
                canvas->drawLine(0.f, -kPadY, 0.f, columnH + kPadY, outlinePaint);
                canvas->translate(kPadX / 2, 0.f);
                static const char kFillLabel[] = "Filled";
                static const char kStrokeLabel[] = "Stroked";
                SkScalar y = columnH + kPadY / 2;
                SkScalar fillX = -outlinePaint.measureText(kFillLabel, strlen(kFillLabel)) - kPadX;
                SkScalar strokeX = kPadX;
                canvas->drawText(kFillLabel, strlen(kFillLabel), fillX, y, labelPaint);
                canvas->drawText(kStrokeLabel, strlen(kStrokeLabel), strokeX, y, labelPaint);
            }
        }
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // disable 565 for now, til mike fixes the debug assert
        return this->INHERITED::onGetFlags() | kSkip565_Flag;
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#ifndef SK_BUILD_FOR_ANDROID
static GM* MyFactory(void*) { return new ShaderText2GM; }
static GMRegistry reg(MyFactory);
#endif
}
