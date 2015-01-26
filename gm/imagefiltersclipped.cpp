/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkColor.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkGradientShader.h"
#include "SkMatrixImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkRectShaderImageFilter.h"
#include "SkScalar.h"
#include "gm.h"

#define RESIZE_FACTOR_X SkIntToScalar(2)
#define RESIZE_FACTOR_Y SkIntToScalar(5)

namespace skiagm {

class ImageFiltersClippedGM : public GM {
public:
    ImageFiltersClippedGM() : fInitialized(false) {
        this->setBGColor(0x00000000);
    }

protected:

    SkString onShortName() SK_OVERRIDE {
        return SkString("imagefiltersclipped");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(860, 500);
    }

    void make_gradient_circle(int width, int height) {
        SkScalar x = SkIntToScalar(width / 2);
        SkScalar y = SkIntToScalar(height / 2);
        SkScalar radius = SkMinScalar(x, y) * 0.8f;
        fGradientCircle.allocN32Pixels(width, height);
        SkCanvas canvas(fGradientCircle);
        canvas.clear(0x00000000);
        SkColor colors[2];
        colors[0] = SK_ColorWHITE;
        colors[1] = SK_ColorBLACK;
        SkAutoTUnref<SkShader> shader(
            SkGradientShader::CreateRadial(SkPoint::Make(x, y), radius, colors, NULL, 2,
                                           SkShader::kClamp_TileMode)
        );
        SkPaint paint;
        paint.setShader(shader);
        canvas.drawCircle(x, y, radius, paint);
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        if (!fInitialized) {
            fCheckerboard.allocN32Pixels(64, 64);
            SkCanvas checkerboardCanvas(fCheckerboard);
            sk_tool_utils::draw_checkerboard(&checkerboardCanvas, 0xFFA0A0A0, 0xFF404040, 8);

            this->make_gradient_circle(64, 64);
            fInitialized = true;
        }
        canvas->clear(0x00000000);

        SkAutoTUnref<SkImageFilter> gradient(SkBitmapSource::Create(fGradientCircle));
        SkAutoTUnref<SkImageFilter> checkerboard(SkBitmapSource::Create(fCheckerboard));
        SkAutoTUnref<SkShader> noise(SkPerlinNoiseShader::CreateFractalNoise(
            SkDoubleToScalar(0.1), SkDoubleToScalar(0.05), 1, 0));
        SkMatrix resizeMatrix;
        resizeMatrix.setScale(RESIZE_FACTOR_X, RESIZE_FACTOR_Y);

        SkImageFilter* filters[] = {
            SkBlurImageFilter::Create(SkIntToScalar(12), SkIntToScalar(12)),
            SkDropShadowImageFilter::Create(SkIntToScalar(10), SkIntToScalar(10),
                SkIntToScalar(3), SkIntToScalar(3), SK_ColorGREEN,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
            SkDisplacementMapEffect::Create(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                            SkDisplacementMapEffect::kR_ChannelSelectorType,
                                            SkIntToScalar(12),
                                            gradient.get(),
                                            checkerboard.get()),
            SkDilateImageFilter::Create(2, 2, checkerboard.get()),
            SkErodeImageFilter::Create(2, 2, checkerboard.get()),
            SkOffsetImageFilter::Create(SkIntToScalar(-16), SkIntToScalar(32)),
            SkMatrixImageFilter::Create(resizeMatrix, SkPaint::kNone_FilterLevel),
            SkRectShaderImageFilter::Create(noise),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar margin = SkIntToScalar(16);
        SkRect bounds = r;
        bounds.outset(margin, margin);

        for (int xOffset = 0; xOffset < 80; xOffset += 16) {
            canvas->save();
            bounds.fLeft = SkIntToScalar(xOffset);
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                paint.setImageFilter(filters[i]);
                paint.setAntiAlias(true);
                canvas->save();
                canvas->clipRect(bounds);
                if (5 == i) {
                    canvas->translate(SkIntToScalar(16), SkIntToScalar(-32));
                } else if (6 == i) {
                    canvas->scale(SkScalarInvert(RESIZE_FACTOR_X),
                                  SkScalarInvert(RESIZE_FACTOR_Y));
                }
                canvas->drawCircle(r.centerX(), r.centerY(),
                                   SkScalarDiv(r.width()*2, SkIntToScalar(5)), paint);
                canvas->restore();
                canvas->translate(r.width() + margin, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() + margin);
        }

        for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
            SkSafeUnref(filters[i]);
        }
    }

private:
    bool fInitialized;
    SkBitmap fCheckerboard;
    SkBitmap fGradientCircle;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ImageFiltersClippedGM; }
static GMRegistry reg(MyFactory);

}
