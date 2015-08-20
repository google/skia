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
#include "SkLightingImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPoint3.h"
#include "SkRectShaderImageFilter.h"
#include "SkScalar.h"
#include "gm.h"

#define RESIZE_FACTOR SkIntToScalar(4)

namespace skiagm {

class ImageFiltersScaledGM : public GM {
public:
    ImageFiltersScaledGM() : fInitialized(false) {
        this->setBGColor(0x00000000);
    }

protected:

    virtual SkString onShortName() {
        return SkString("imagefiltersscaled");
    }

    virtual SkISize onISize() {
        return SkISize::Make(1428, 500);
    }

    void make_gradient_circle(int width, int height) {
        SkScalar x = SkIntToScalar(width / 2);
        SkScalar y = SkIntToScalar(height / 2);
        SkScalar radius = SkScalarMul(SkMinScalar(x, y), SkIntToScalar(4) / SkIntToScalar(5));
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

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            fCheckerboard.allocN32Pixels(64, 64);
            SkCanvas checkerboardCanvas(fCheckerboard);
            sk_tool_utils::draw_checkerboard(&checkerboardCanvas, 0xFFA0A0A0, 0xFF404040, 8);

            this->make_gradient_circle(64, 64);
            fInitialized = true;
        }
        canvas->clear(SK_ColorBLACK);

        SkAutoTUnref<SkImageFilter> gradient(SkBitmapSource::Create(fGradientCircle));
        SkAutoTUnref<SkImageFilter> checkerboard(SkBitmapSource::Create(fCheckerboard));
        SkAutoTUnref<SkShader> noise(SkPerlinNoiseShader::CreateFractalNoise(
            SkDoubleToScalar(0.1), SkDoubleToScalar(0.05), 1, 0));

        SkPoint3 pointLocation = SkPoint3::Make(0, 0, SkIntToScalar(10));
        SkPoint3 spotLocation = SkPoint3::Make(SkIntToScalar(-10), 
                                               SkIntToScalar(-10), 
                                               SkIntToScalar(20));
        SkPoint3 spotTarget = SkPoint3::Make(SkIntToScalar(40), SkIntToScalar(40), 0);
        SkScalar spotExponent = SK_Scalar1;
        SkScalar cutoffAngle = SkIntToScalar(15);
        SkScalar kd = SkIntToScalar(2);
        SkScalar surfaceScale = SkIntToScalar(1);
        SkColor white(0xFFFFFFFF);
        SkMatrix resizeMatrix;
        resizeMatrix.setScale(RESIZE_FACTOR, RESIZE_FACTOR);

        SkImageFilter* filters[] = {
            SkBlurImageFilter::Create(SkIntToScalar(4), SkIntToScalar(4)),
            SkDropShadowImageFilter::Create(SkIntToScalar(5), SkIntToScalar(10),
                SkIntToScalar(3), SkIntToScalar(3), SK_ColorYELLOW,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
            SkDisplacementMapEffect::Create(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                            SkDisplacementMapEffect::kR_ChannelSelectorType,
                                            SkIntToScalar(12),
                                            gradient.get(),
                                            checkerboard.get()),
            SkDilateImageFilter::Create(1, 1, checkerboard.get()),
            SkErodeImageFilter::Create(1, 1, checkerboard.get()),
            SkOffsetImageFilter::Create(SkIntToScalar(32), 0),
            SkImageFilter::CreateMatrixFilter(resizeMatrix, kNone_SkFilterQuality),
            SkRectShaderImageFilter::Create(noise),
            SkLightingImageFilter::CreatePointLitDiffuse(pointLocation, white, surfaceScale, kd),
            SkLightingImageFilter::CreateSpotLitDiffuse(spotLocation, spotTarget, spotExponent,
                                                        cutoffAngle, white, surfaceScale, kd),
        };

        SkVector scales[] = {
            SkVector::Make(SkScalarInvert(2), SkScalarInvert(2)),
            SkVector::Make(SkIntToScalar(1), SkIntToScalar(1)),
            SkVector::Make(SkIntToScalar(1), SkIntToScalar(2)),
            SkVector::Make(SkIntToScalar(2), SkIntToScalar(1)),
            SkVector::Make(SkIntToScalar(2), SkIntToScalar(2)),
        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar margin = SkIntToScalar(16);
        SkRect bounds = r;
        bounds.outset(margin, margin);

        for (size_t j = 0; j < SK_ARRAY_COUNT(scales); ++j) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkPaint paint;
                paint.setColor(SK_ColorBLUE);
                paint.setImageFilter(filters[i]);
                paint.setAntiAlias(true);
                canvas->save();
                canvas->scale(scales[j].fX, scales[j].fY);
                if (5 == i) {
                    canvas->translate(SkIntToScalar(-32), 0);
                } else if (6 == i) {
                    canvas->scale(SkScalarInvert(RESIZE_FACTOR),
                                  SkScalarInvert(RESIZE_FACTOR));
                }
                canvas->drawCircle(r.centerX(), r.centerY(), r.width()*2/5, paint);
                canvas->restore();
                canvas->translate(r.width() * scales[j].fX + margin, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() * scales[j].fY + margin);
        }

        for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
            filters[i]->unref();
        }
    }

private:
    bool fInitialized;
    SkBitmap fCheckerboard;
    SkBitmap fGradientCircle;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ImageFiltersScaledGM; }
static GMRegistry reg(MyFactory);

}
