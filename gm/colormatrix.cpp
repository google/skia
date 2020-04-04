/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"

#define WIDTH 500
#define HEIGHT 160

static void set_color_matrix(SkPaint* paint, const SkColorMatrix& matrix) {
    paint->setColorFilter(SkColorFilters::Matrix(matrix));
}

static void set_array(SkPaint* paint, const float array[]) {
    paint->setColorFilter(SkColorFilters::Matrix(array));
}

class ColorMatrixGM : public skiagm::GM {
public:
    ColorMatrixGM() {
        this->setBGColor(0xFF808080);
    }

protected:
    SkString onShortName() override {
        return SkString("colormatrix");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onOnceBeforeDraw() override {
        fSolidImg = CreateSolidBitmap(64, 64);
        fTransparentImg = CreateTransparentBitmap(64, 64);
    }

    static sk_sp<SkImage> CreateSolidBitmap(int width, int height) {
        SkBitmap bm;
        bm.allocN32Pixels(width, height);
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
        return SkImage::MakeFromBitmap(bm);
    }

    // creates a bitmap with shades of transparent gray.
    static sk_sp<SkImage> CreateTransparentBitmap(int width, int height) {
        SkBitmap bm;
        bm.allocN32Pixels(width, height);
        SkCanvas canvas(bm);
        canvas.clear(0x0);

        SkPoint pts[] = {{0, 0}, {SkIntToScalar(width), SkIntToScalar(height)}};
        SkColor colors[] = {0x00000000, 0xFFFFFFFF};
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                     SkTileMode::kClamp));
        canvas.drawRect(SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height)), paint);
        return SkImage::MakeFromBitmap(bm);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        SkColorMatrix matrix;

        paint.setBlendMode(SkBlendMode::kSrc);
        const SkImage* bmps[] = { fSolidImg.get(), fTransparentImg.get() };

        for (size_t i = 0; i < SK_ARRAY_COUNT(bmps); ++i) {
            matrix.setIdentity();
            set_color_matrix(&paint, matrix);
            canvas->drawImage(bmps[i], 0, 0, &paint);

            ///////////////////////////////////////////////

            matrix.setSaturation(0.0f);
            set_color_matrix(&paint, matrix);
            canvas->drawImage(bmps[i], 80, 0, &paint);

            matrix.setSaturation(0.5f);
            set_color_matrix(&paint, matrix);
            canvas->drawImage(bmps[i], 160, 0, &paint);

            matrix.setSaturation(1.0f);
            set_color_matrix(&paint, matrix);
            canvas->drawImage(bmps[i], 240, 0, &paint);

            matrix.setSaturation(2.0f);
            set_color_matrix(&paint, matrix);
            canvas->drawImage(bmps[i], 320, 0, &paint);

            ///////////////////////////////////////////////

            // Move red into alpha, set color to white
            float data[20] = {
                0,  0, 0, 0, 1,
                0,  0, 0, 0, 1,
                0,  0, 0, 0, 1,
                1, 0, 0, 0, 0,
            };

            set_array(&paint, data);
            canvas->drawImage(bmps[i], 400, 0, &paint);
            ///////////////////////////////////////////////
            canvas->translate(0, 80);
        }
    }

private:
    sk_sp<SkImage>   fSolidImg;
    sk_sp<SkImage>   fTransparentImg;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ColorMatrixGM; )
