/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"

static void makebm(SkBitmap* bm, int w, int h) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(SK_ColorTRANSPARENT);

    SkCanvas    canvas(*bm);
    SkScalar    s = SkIntToScalar(SkMin32(w, h));
    const SkPoint     kPts0[] = { { 0, 0 }, { s, s } };
    const SkPoint     kPts1[] = { { s, 0 }, { 0, s } };
    const SkScalar    kPos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    const SkColor kColors0[] = {0x40FF00FF, 0xF0FFFF00, 0x4000FFFF };
    const SkColor kColors1[] = {0xF0FF00FF, 0x80FFFF00, 0xF000FFFF };


    SkPaint     paint;

    paint.setShader(SkGradientShader::MakeLinear(kPts0, kColors0, kPos,
                    SK_ARRAY_COUNT(kColors0), SkShader::kClamp_TileMode));
    canvas.drawPaint(paint);
    paint.setShader(SkGradientShader::MakeLinear(kPts1, kColors1, kPos,
                    SK_ARRAY_COUNT(kColors1), SkShader::kClamp_TileMode));
    canvas.drawPaint(paint);
}

///////////////////////////////////////////////////////////////////////////////

struct LabeledMatrix {
    SkMatrix    fMatrix;
    const char* fLabel;
};

DEF_SIMPLE_GM_BG(shadertext2, canvas, 1800, 900,
                 sk_tool_utils::color_to_565(0xFFDDDDDD)) {
        constexpr char kText[] = "SKIA";
        constexpr int kTextLen = SK_ARRAY_COUNT(kText) - 1;
        constexpr int kPointSize = 55;

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
            makebm(&bmp, kPointSize / 2, kPointSize / 2);
        }

        SkPaint fillPaint;
        fillPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&fillPaint);
        fillPaint.setTextSize(SkIntToScalar(kPointSize));
        fillPaint.setFilterQuality(kLow_SkFilterQuality);

        SkPaint outlinePaint;
        outlinePaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&outlinePaint);
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
        sk_tool_utils::set_portable_typeface(&labelPaint);
        labelPaint.setTextSize(12.f);

        canvas->translate(15.f, 15.f);
        canvas->drawBitmap(bmp, 0, 0);
        canvas->translate(0, bmp.height() + labelPaint.getTextSize() + 15.f);

        constexpr char kLabelLabel[] = "localM / canvasM";
        canvas->drawString(kLabelLabel, 0, 0, labelPaint);
        canvas->translate(0, 15.f);

        canvas->save();
        SkScalar maxLabelW = 0;
        canvas->translate(0, kPadY / 2 + kPointSize);
        for (int lm = 0; lm < localMatrices.count(); ++lm) {
            canvas->drawString(matrices[lm].fLabel,
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

            SkScalar columnH = 0;
            for (int m = 0; m < matrices.count(); ++m) {
                columnH = 0;
                canvas->save();
                canvas->drawString(matrices[m].fLabel,
                                 0, labelPaint.getTextSize() - 1, labelPaint);
                canvas->translate(0, kPadY / 2 + kPointSize);
                columnH += kPadY / 2 + kPointSize;
                for (int lm = 0; lm < localMatrices.count(); ++lm) {
                    paint.setShader(SkShader::MakeBitmapShader(bmp, SkShader::kMirror_TileMode,
                                                               SkShader::kRepeat_TileMode,
                                                               &localMatrices[lm].fMatrix));

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
                        canvas->drawTextOnPath(kText, kTextLen, path, nullptr, paint);
                        canvas->drawTextOnPath(kText, kTextLen, path, nullptr, outlinePaint);
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
                constexpr char kFillLabel[] = "Filled";
                constexpr char kStrokeLabel[] = "Stroked";
                SkScalar y = columnH + kPadY / 2;
                SkScalar fillX = -outlinePaint.measureText(kFillLabel, strlen(kFillLabel)) - kPadX;
                SkScalar strokeX = kPadX;
                canvas->drawString(kFillLabel, fillX, y, labelPaint);
                canvas->drawString(kStrokeLabel, strokeX, y, labelPaint);
            }
        }
}
