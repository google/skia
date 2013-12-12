/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorMatrixFilter.h"
#include "SkGradientShader.h"

#define WIDTH 500
#define HEIGHT 500

class SkOnce {
public:
    SkOnce() : fOnce(false) {};

    bool once() const {
        if (fOnce) {
            return false;
        }
        fOnce = true;
        return true;
    }

private:
    mutable bool fOnce;
};

static void setColorMatrix(SkPaint* paint, const SkColorMatrix& matrix) {
    paint->setColorFilter(SkNEW_ARGS(SkColorMatrixFilter, (matrix)))->unref();
}

static void setArray(SkPaint* paint, const SkScalar array[]) {
    paint->setColorFilter(SkNEW_ARGS(SkColorMatrixFilter, (array)))->unref();
}

namespace skiagm {

class ColorMatrixGM : public GM {
    SkOnce fOnce;
    void init() {
        if (fOnce.once()) {
            fSolidBitmap = this->createSolidBitmap(64, 64);
            fTransparentBitmap = this->createTransparentBitmap(64, 64);
        }
    }

public:
    ColorMatrixGM() {
        this->setBGColor(0xFF808080);
    }

protected:
    virtual SkString onShortName() {
        return SkString("colormatrix");
    }

    virtual SkISize onISize() {
        return make_isize(WIDTH, HEIGHT);
    }

    SkBitmap createSolidBitmap(int width, int height) {
        SkBitmap bm;
        bm.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bm.allocPixels();
        SkCanvas canvas(bm);
        canvas.clear(0x0);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                SkPaint paint;
                paint.setColor(SkColorSetARGB(255, x * 255 / width, y * 255 / height, 0));
                canvas.drawRect(SkRect::MakeXYWH(SkIntToScalar(x),
                    SkIntToScalar(y), SK_Scalar1, SK_Scalar1), paint);
            }
        }
        return bm;
    }

    // creates a bitmap with shades of transparent gray.
    SkBitmap createTransparentBitmap(int width, int height) {
        SkBitmap bm;
        bm.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bm.allocPixels();
        SkCanvas canvas(bm);
        canvas.clear(0x0);

        SkPoint pts[] = {{0, 0}, {SkIntToScalar(width), SkIntToScalar(height)}};
        SkColor colors[] = {0x00000000, 0xFFFFFFFF};
        SkPaint paint;
        paint.setShader(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                       SkShader::kClamp_TileMode))->unref();
        canvas.drawRect(SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height)), paint);
        return bm;
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->init();

        SkPaint paint;
        SkColorMatrix matrix;

        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        const SkBitmap bmps[] = { fSolidBitmap, fTransparentBitmap };

        for (size_t i = 0; i < SK_ARRAY_COUNT(bmps); ++i) {

            matrix.setIdentity();
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 0, 0, &paint);

            matrix.setRotate(SkColorMatrix::kR_Axis, 90);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 80, 0, &paint);

            matrix.setRotate(SkColorMatrix::kG_Axis, 90);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 160, 0, &paint);

            matrix.setRotate(SkColorMatrix::kB_Axis, 90);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 240, 0, &paint);

            matrix.setSaturation(0.0f);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 0, 80, &paint);

            matrix.setSaturation(0.5f);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 80, 80, &paint);

            matrix.setSaturation(1.0f);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 160, 80, &paint);

            matrix.setSaturation(2.0f);
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 240, 80, &paint);

            matrix.setRGB2YUV();
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 0, 160, &paint);

            matrix.setYUV2RGB();
            setColorMatrix(&paint, matrix);
            canvas->drawBitmap(bmps[i], 80, 160, &paint);

            SkScalar s1 = SK_Scalar1;
            SkScalar s255 = SkIntToScalar(255);
            // Move red into alpha, set color to white
            SkScalar data[20] = {
                0,  0, 0, 0, s255,
                0,  0, 0, 0, s255,
                0,  0, 0, 0, s255,
                s1, 0, 0, 0, 0,
            };

            setArray(&paint, data);
            canvas->drawBitmap(bmps[i], 160, 160, &paint);

            canvas->translate(0, 240);
        }
    }

private:
    SkBitmap fSolidBitmap;
    SkBitmap fTransparentBitmap;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ColorMatrixGM; }
static GMRegistry reg(MyFactory);

}
