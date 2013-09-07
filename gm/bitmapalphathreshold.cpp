/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkGradientShader.h"
#include "SkBitmapAlphaThresholdShader.h"
#include "SkTArray.h"
#include "SkParsePath.h"

class BitmapAlphaThresholdGM : public skiagm::GM {
public:
    BitmapAlphaThresholdGM() {
        this->setBGColor(0xFF000000);
    }

private:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // narrow this flags when the shader has a CPU implementation and
        // when it serializes.
        return
            kSkipPDF_Flag               |
            kSkipPicture_Flag           |
            kSkipPipe_Flag              |
            kSkipPipeCrossProcess_Flag  |
            kSkipTiled_Flag             |
            kSkip565_Flag               |
            kSkipScaledReplay_Flag      |
            kSkipPDFRasterization_Flag  |

            kGPUOnly_Flag;
    }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        fBM.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
        if (!fBM.allocPixels()) {
            return;
        }
        SkCanvas canvas(fBM);
        SkPoint pts[] = { {0, 0}, {SkIntToScalar(fBM.width()), SkIntToScalar(fBM.height())} };
        SkColor colors[] = {0x00000000, 0xffffffff};
        SkShader* grad = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                        SkShader::kClamp_TileMode);
        SkPaint gradPaint;
        gradPaint.setShader(grad)->unref();
        gradPaint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.drawPaint(gradPaint);

        // Construct the region used as a mask.
        SkRegion bmpBoundsClip;
        bmpBoundsClip.setRect(0, 0, fBM.width(), fBM.height());
        SkPath circlePath;
        SkScalar radius = SkScalarSqrt(SkIntToScalar(fBM.width() * fBM.height())) / 2;
        circlePath.addCircle(SkIntToScalar(fBM.width() / 2),
                             SkIntToScalar(fBM.height() / 2),
                             radius);
        fMask.setPath(circlePath, bmpBoundsClip);

        SkPath batPath;
        SkParsePath::FromSVGString(
        "M305.214,374.779c2.463,0,3.45,0.493,3.45,0.493l1.478-6.241c0,0,1.15,4.763,1.643,9.034"
        "c0.493,4.271,8.048,1.479,14.454,0.164c6.405-1.314,7.72-11.662,7.72-11.662h59.294c0,0-35.807,10.841-26.772,34.656"
        "c0,0-52.889-8.048-61.101,24.967h-0.001c-8.212-33.015-61.101-24.967-61.101-24.967c9.034-23.815-26.772-34.656-26.772-34.656"
        "h59.294c0,0,1.314,10.348,7.719,11.662c6.406,1.314,13.962,4.106,14.454-0.164c0.493-4.271,1.643-9.034,1.643-9.034l1.479,6.241"
        "c0,0,0.985-0.493,3.449-0.493H305.214L305.214,374.779z",
        &batPath);

        SkMatrix matrix;
        matrix.setTranslate(-208, -280);
        matrix.postScale(radius / 100, radius / 100);
        batPath.transform(matrix, &batPath);
        SkRegion batRegion;
        batRegion.setPath(batPath, bmpBoundsClip);

        fMask.op(batRegion, SkRegion::kDifference_Op);
    }

    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("bat");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(518, 735);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkTArray<SkMatrix> lms;
        lms.push_back().reset();
        lms.push_back().setScale(SK_Scalar1 / 2, SK_Scalar1);
        lms.push_back().setScale(SK_Scalar1, 2 * SK_Scalar1);
        lms.push_back().setRotate(-SK_Scalar1 * 30);
        lms.push_back().setSkew(0, SK_Scalar1 / 5);

        static const SkScalar kMargin = 5 * SK_Scalar1;
        canvas->translate(kMargin, kMargin);
        canvas->save();

        static const U8CPU kThresholds[] = { 0x0, 0x08, 0x40, 0x80, 0xC0, 0xF0, 0xFF };

        for (size_t i = 0; i < SK_ARRAY_COUNT(kThresholds); ++i) {
            for (int j = 0; j < lms.count(); ++j) {
                SkRect rect;
                rect.fLeft = 0;
                rect.fTop = 0;
                rect.fRight = SkIntToScalar(fBM.width());
                rect.fBottom = SkIntToScalar(fBM.height());

                SkShader* thresh;
                // This SkShader currently only has a GPU implementation.
                if (canvas->getDevice()->accessRenderTarget()) {
                    thresh = SkBitmapAlphaThresholdShader::Create(fBM, fMask, kThresholds[i]);
                } else {
                    thresh = SkShader::CreateBitmapShader(fBM, SkShader::kClamp_TileMode,
                                                               SkShader::kClamp_TileMode);
                }

                thresh->setLocalMatrix(lms[j]);

                SkPaint paint;
                paint.setShader(thresh)->unref();

                canvas->drawRect(rect, paint);
                canvas->translate(SkIntToScalar(fBM.width() + kMargin), 0);
            }
            canvas->restore();
            canvas->translate(0, SkIntToScalar(fBM.height() + kMargin));
            canvas->save();
        }

    }

    SkBitmap    fBM;
    SkRegion    fMask;

    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new BitmapAlphaThresholdGM(); )
