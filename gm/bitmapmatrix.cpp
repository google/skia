
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBitmap.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkSize.h"
#include "SkString.h"

namespace skiagm {

class DrawBitmapMatrixGM : public GM {
public:
    DrawBitmapMatrixGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("drawbitmapmatrix");
    }

    virtual SkISize onISize() SK_OVERRIDE { return make_isize(1024, 256); }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkBitmap bm;
        this->setupBitmap(&bm);

        // Draw normally.
        SkMatrix matrix;
        matrix.reset();
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setDither(true);
        canvas->drawBitmapMatrix(bm, matrix, &paint);

        // Draw stretched horizontally and squished vertically.
        canvas->translate(SkIntToScalar(bm.width() + 5), 0);
        matrix.setScale(SkIntToScalar(2), SK_ScalarHalf);
        canvas->drawBitmapMatrix(bm, matrix, &paint);

        // Draw rotated
        canvas->translate(SkIntToScalar(bm.width()*2 + 5), 0);
        matrix.reset();
        matrix.setRotate(SkIntToScalar(45), SkIntToScalar(bm.width() / 2),
                         SkIntToScalar(bm.height() / 2));
        canvas->save();
        canvas->translate(0, SkIntToScalar(10));
        canvas->drawBitmapMatrix(bm, matrix, &paint);
        canvas->restore();

        // Draw with perspective
        canvas->translate(SkIntToScalar(bm.width() + 15), 0);
        matrix.reset();
        matrix.setPerspX(SkScalarDiv(SK_Scalar1, SkIntToScalar(1000)));
        matrix.setPerspY(SkScalarDiv(SK_Scalar1, SkIntToScalar(1000)));
        canvas->drawBitmapMatrix(bm, matrix, &paint);

        // Draw with skew
        canvas->translate(SkIntToScalar(bm.width() + 5), 0);
        matrix.reset();
        matrix.setSkew(SkIntToScalar(2), SkIntToScalar(2));
        canvas->drawBitmapMatrix(bm, matrix, &paint);

        // Draw with sin/cos
        canvas->translate(SkIntToScalar(bm.width() * 4), 0);
        matrix.reset();
        matrix.setSinCos(SK_ScalarHalf, SkIntToScalar(2));
        canvas->drawBitmapMatrix(bm, matrix, &paint);

        {
            // test the following code path:
            // SkGpuDevice::drawPath() -> SkGpuDevice::drawWithMaskFilter()
            SkPaint paint;

            paint.setFilterLevel(SkPaint::kLow_FilterLevel);

            SkMaskFilter* mf = SkBlurMaskFilter::Create(
                SkBlurMaskFilter::kNormal_BlurStyle,
                SkBlurMask::ConvertRadiusToSigma(5),
                SkBlurMaskFilter::kHighQuality_BlurFlag |
                SkBlurMaskFilter::kIgnoreTransform_BlurFlag);
            paint.setMaskFilter(mf)->unref();

            canvas->translate(SkIntToScalar(bm.width()*2 + 20), 0);

            matrix.reset();
            matrix.setRotate(SkIntToScalar(45), SkIntToScalar(bm.width() / 2),
                             SkIntToScalar(bm.height() / 2));

            canvas->save();
            canvas->translate(0, SkIntToScalar(20));
            canvas->drawBitmapMatrix(bm, matrix, &paint);
            canvas->restore();
        }

    }
private:
    void setupBitmap(SkBitmap* bm) {
        SkASSERT(bm);
        static const int SIZE = 64;
        bm->setConfig(SkBitmap::kARGB_8888_Config, SIZE, SIZE);
        bm->allocPixels();
        SkCanvas canvas(*bm);

        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        canvas.drawPaint(paint);

        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);
        SkRect rect = SkRect::MakeWH(SkIntToScalar(SIZE), SkIntToScalar(SIZE));
        SkPath path;
        path.addOval(rect);
        canvas.drawPath(path, paint);
    }
};

////////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DrawBitmapMatrixGM; }
static GMRegistry reg(MyFactory);

}
