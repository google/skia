/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "tools/ToolUtils.h"

#include <utility>

#define RESIZE_FACTOR SkIntToScalar(4)

static sk_sp<SkImage> make_gradient_circle(int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = std::min(x, y) * 4 / 5;
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height)));
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
    SkString getName() const override { return SkString("imagefiltersscaled"); }

    SkISize getISize() override { return SkISize::Make(1428, 500); }

    void onOnceBeforeDraw() override {
        fCheckerboard = ToolUtils::create_checkerboard_image(64, 64, 0xFFA0A0A0, 0xFF404040, 8);
        fGradientCircle = make_gradient_circle(64, 64);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        sk_sp<SkImageFilter> gradient(SkImageFilters::Image(fGradientCircle,
                                                            SkFilterMode::kLinear));
        sk_sp<SkImageFilter> checkerboard(SkImageFilters::Image(fCheckerboard,
                                                                SkFilterMode::kLinear));

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
                SkImageFilters::Blur(SkIntToScalar(4), SkIntToScalar(4), nullptr),
                SkImageFilters::DropShadow(5, 10, 3, 3, SK_ColorYELLOW, nullptr),
                SkImageFilters::DisplacementMap(SkColorChannel::kR,
                                                SkColorChannel::kR,
                                                12,
                                                std::move(gradient),
                                                checkerboard),
                SkImageFilters::Dilate(1, 1, checkerboard),
                SkImageFilters::Erode(1, 1, checkerboard),
                SkImageFilters::Offset(SkIntToScalar(32), 0, nullptr),
                SkImageFilters::MatrixTransform(resizeMatrix, SkSamplingOptions(), nullptr),
                SkImageFilters::Shader(SkShaders::MakeFractalNoise(
                        SkDoubleToScalar(0.1), SkDoubleToScalar(0.05), 1, 0)),
                SkImageFilters::PointLitDiffuse(pointLocation, white, surfaceScale, kd, nullptr),
                SkImageFilters::SpotLitDiffuse(spotLocation,
                                               spotTarget,
                                               spotExponent,
                                               cutoffAngle,
                                               white,
                                               surfaceScale,
                                               kd,
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

        for (size_t j = 0; j < std::size(scales); ++j) {
            canvas->save();
            for (size_t i = 0; i < std::size(filters); ++i) {
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

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFiltersScaledGM;)
}  // namespace skiagm
