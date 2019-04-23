/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageSource.h"
#include "include/effects/SkLightingImageFilter.h"
#include "include/effects/SkMorphologyImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"
#include "include/effects/SkPaintImageFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "tools/ToolUtils.h"

#define RESIZE_FACTOR SkIntToScalar(4)

static sk_sp<SkImage> make_gradient_circle(int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = SkMinScalar(x, y) * 4 / 5;
    sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(width, height));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0x00000000);
    SkColor colors[2];
    colors[0] = SK_ColorWHITE;
    colors[1] = SK_ColorBLACK;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(x, y), radius, colors, nullptr,
        2, SkTileMode::kClamp));
    canvas->drawCircle(x, y, radius, paint);

    return surface->makeImageSnapshot();
}


namespace skiagm {

class ImageFiltersScaledGM : public GM {
public:
    ImageFiltersScaledGM() {
        this->setBGColor(0x00000000);
    }

protected:

    SkString onShortName() override {
        return SkString("imagefiltersscaled");
    }

    SkISize onISize() override {
        return SkISize::Make(1428, 500);
    }

    void onOnceBeforeDraw() override {
        fCheckerboard = SkImage::MakeFromBitmap(
                ToolUtils::create_checkerboard_bitmap(64, 64, 0xFFA0A0A0, 0xFF404040, 8));
        fGradientCircle = make_gradient_circle(64, 64);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        sk_sp<SkImageFilter> gradient(SkImageSource::Make(fGradientCircle));
        sk_sp<SkImageFilter> checkerboard(SkImageSource::Make(fCheckerboard));

        SkPaint noisePaint;
        noisePaint.setShader(SkPerlinNoiseShader::MakeFractalNoise(SkDoubleToScalar(0.1),
                                                                   SkDoubleToScalar(0.05), 1, 0));

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

        sk_sp<SkImageFilter> filters[] = {
            SkBlurImageFilter::Make(SkIntToScalar(4), SkIntToScalar(4), nullptr),
            SkDropShadowImageFilter::Make(
                                    SkIntToScalar(5), SkIntToScalar(10),
                                    SkIntToScalar(3), SkIntToScalar(3), SK_ColorYELLOW,
                                    SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                                    nullptr),
            SkDisplacementMapEffect::Make(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                          SkDisplacementMapEffect::kR_ChannelSelectorType,
                                          SkIntToScalar(12),
                                          std::move(gradient),
                                          checkerboard),
            SkDilateImageFilter::Make(1, 1, checkerboard),
            SkErodeImageFilter::Make(1, 1, checkerboard),
            SkOffsetImageFilter::Make(SkIntToScalar(32), 0, nullptr),
            SkImageFilter::MakeMatrixFilter(resizeMatrix, kNone_SkFilterQuality, nullptr),
            SkPaintImageFilter::Make(noisePaint),
            SkLightingImageFilter::MakePointLitDiffuse(pointLocation, white, surfaceScale, kd,
                                                       nullptr),
            SkLightingImageFilter::MakeSpotLitDiffuse(spotLocation, spotTarget, spotExponent,
                                                      cutoffAngle, white, surfaceScale, kd,
                                                      nullptr),
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
                canvas->clipRect(r);
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
    }

private:
    sk_sp<SkImage> fCheckerboard, fGradientCircle;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFiltersScaledGM;)
}
