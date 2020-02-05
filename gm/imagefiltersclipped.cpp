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
#include "include/core/SkFilterQuality.h"
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

#define RESIZE_FACTOR_X SkIntToScalar(2)
#define RESIZE_FACTOR_Y SkIntToScalar(5)

static sk_sp<SkImage> make_gradient_circle(int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = std::min(x, y) * 0.8f;
    auto surface(SkSurface::MakeRasterN32Premul(width, height));
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

static void draw_clipped_filter(SkCanvas* canvas, sk_sp<SkImageFilter> filter, size_t i,
                                const SkRect& primBounds, const SkRect& clipBounds) {
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setImageFilter(std::move(filter));
    paint.setAntiAlias(true);
    canvas->save();
    canvas->clipRect(clipBounds);
    if (5 == i) {
        canvas->translate(SkIntToScalar(16), SkIntToScalar(-32));
    } else if (6 == i) {
        canvas->scale(SkScalarInvert(RESIZE_FACTOR_X), SkScalarInvert(RESIZE_FACTOR_Y));
    }
    canvas->drawCircle(primBounds.centerX(), primBounds.centerY(),
                       primBounds.width() * 2 / 5, paint);
    canvas->restore();
}

namespace skiagm {

class ImageFiltersClippedGM : public GM {
public:
    ImageFiltersClippedGM() {
        this->setBGColor(0x00000000);
    }

protected:
    SkString onShortName() override {
        return SkString("imagefiltersclipped");
    }

    SkISize onISize() override {
        return SkISize::Make(860, 500);
    }

    void onOnceBeforeDraw() override {
        fCheckerboard = SkImage::MakeFromBitmap(
                ToolUtils::create_checkerboard_bitmap(64, 64, 0xFFA0A0A0, 0xFF404040, 8));
        fGradientCircle = make_gradient_circle(64, 64);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        sk_sp<SkImageFilter> gradient(SkImageFilters::Image(fGradientCircle));
        sk_sp<SkImageFilter> checkerboard(SkImageFilters::Image(fCheckerboard));
        SkMatrix resizeMatrix;
        resizeMatrix.setScale(RESIZE_FACTOR_X, RESIZE_FACTOR_Y);
        SkPoint3 pointLocation = SkPoint3::Make(32, 32, SkIntToScalar(10));

        sk_sp<SkImageFilter> filters[] = {
            SkImageFilters::Blur(SkIntToScalar(12), SkIntToScalar(12), nullptr),
            SkImageFilters::DropShadow(SkIntToScalar(10), SkIntToScalar(10),
                                       SkIntToScalar(3), SkIntToScalar(3), SK_ColorGREEN, nullptr),
            SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kR,
                                            SkIntToScalar(12), std::move(gradient), checkerboard),
            SkImageFilters::Dilate(2, 2, checkerboard),
            SkImageFilters::Erode(2, 2, checkerboard),
            SkImageFilters::Offset(SkIntToScalar(-16), SkIntToScalar(32), nullptr),
            SkImageFilters::MatrixTransform(resizeMatrix, kNone_SkFilterQuality, nullptr),
            SkImageFilters::PointLitDiffuse(pointLocation, SK_ColorWHITE, SK_Scalar1,
                                            SkIntToScalar(2), checkerboard),

        };

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar margin = SkIntToScalar(16);
        SkRect bounds = r;
        bounds.outset(margin, margin);

        canvas->save();
        for (int xOffset = 0; xOffset < 80; xOffset += 16) {
            canvas->save();
            bounds.fLeft = SkIntToScalar(xOffset);
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                draw_clipped_filter(canvas, filters[i], i, r, bounds);
                canvas->translate(r.width() + margin, 0);
            }
            canvas->restore();
            canvas->translate(0, r.height() + margin);
        }
        canvas->restore();

        SkPaint noisePaint;
        noisePaint.setShader(SkPerlinNoiseShader::MakeFractalNoise(0.1f, 0.05f, 1, 0));

        sk_sp<SkImageFilter> rectFilter(SkImageFilters::Paint(noisePaint));
        canvas->translate(SK_ARRAY_COUNT(filters)*(r.width() + margin), 0);
        for (int xOffset = 0; xOffset < 80; xOffset += 16) {
            bounds.fLeft = SkIntToScalar(xOffset);
            draw_clipped_filter(canvas, rectFilter, 0, r, bounds);
            canvas->translate(0, r.height() + margin);
        }
    }

private:
    sk_sp<SkImage> fCheckerboard, fGradientCircle;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFiltersClippedGM;)
}
