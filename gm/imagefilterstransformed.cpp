/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
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
#include "tools/ToolUtils.h"

#include <utility>

namespace skiagm {

// This GM draws image filters with a CTM containing shearing / rotation.
// It checks that the scale portion of the CTM is correctly extracted
// and applied to the image inputs separately from the non-scale portion.

static sk_sp<SkImage> make_gradient_circle(int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = SkMinScalar(x, y) * 0.8f;

    auto surface(SkSurface::MakeRasterN32Premul(width, height));
    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(0x00000000);
    SkColor colors[2];
    colors[0] = SK_ColorWHITE;
    colors[1] = SK_ColorBLACK;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(SkPoint::Make(x, y), radius, colors, nullptr, 2,
                                                 SkTileMode::kClamp));
    canvas->drawCircle(x, y, radius, paint);

    return surface->makeImageSnapshot();
}

class ImageFiltersTransformedGM : public GM {
public:
    ImageFiltersTransformedGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:

    SkString onShortName() override { return SkString("imagefilterstransformed"); }

    SkISize onISize() override { return SkISize::Make(420, 240); }

    void onOnceBeforeDraw() override {
        fCheckerboard = SkImage::MakeFromBitmap(
                ToolUtils::create_checkerboard_bitmap(64, 64, 0xFFA0A0A0, 0xFF404040, 8));
        fGradientCircle = make_gradient_circle(64, 64);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkImageFilter> gradient(SkImageFilters::Image(fGradientCircle));
        sk_sp<SkImageFilter> checkerboard(SkImageFilters::Image(fCheckerboard));
        sk_sp<SkImageFilter> filters[] = {
            SkImageFilters::Blur(12, 0, nullptr),
            SkImageFilters::DropShadow(0, 15, 8, 0, SK_ColorGREEN, nullptr),
            SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kR, 12,
                                            std::move(gradient), checkerboard),
            SkImageFilters::Dilate(2, 2, checkerboard),
            SkImageFilters::Erode(2, 2, checkerboard),
        };

        const SkScalar margin = SkIntToScalar(20);
        const SkScalar size = SkIntToScalar(60);

        for (size_t j = 0; j < 3; j++) {
            canvas->save();
            canvas->translate(margin, 0);
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkPaint paint;
                paint.setColor(SK_ColorWHITE);
                paint.setImageFilter(filters[i]);
                paint.setAntiAlias(true);
                canvas->save();
                canvas->translate(size * SK_ScalarHalf, size * SK_ScalarHalf);
                canvas->scale(SkDoubleToScalar(0.8), SkDoubleToScalar(0.8));
                if (j == 1) {
                    canvas->rotate(SkIntToScalar(45));
                } else if (j == 2) {
                    canvas->skew(SkDoubleToScalar(0.5), SkDoubleToScalar(0.2));
                }
                canvas->translate(-size * SK_ScalarHalf, -size * SK_ScalarHalf);
                canvas->drawOval(SkRect::MakeXYWH(0, size * SkDoubleToScalar(0.1),
                                                  size, size * SkDoubleToScalar(0.6)), paint);
                canvas->restore();
                canvas->translate(size + margin, 0);
            }
            canvas->restore();
            canvas->translate(0, size + margin);
        }
    }

private:
    sk_sp<SkImage> fCheckerboard;
    sk_sp<SkImage> fGradientCircle;
    typedef GM INHERITED;
};
DEF_GM( return new ImageFiltersTransformedGM; )
}

//////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(rotate_imagefilter, canvas, 500, 500) {
    SkPaint paint;

    const SkRect r = SkRect::MakeXYWH(50, 50, 100, 100);

    sk_sp<SkImageFilter> filters[] = {
        nullptr,
        SkImageFilters::Blur(6, 0, nullptr),
        SkImageFilters::Xfermode(SkBlendMode::kSrcOver, nullptr),
    };

    for (auto& filter : filters) {
        paint.setAntiAlias(false);
        paint.setImageFilter(filter);

        canvas->save();

        canvas->drawRect(r, paint);

        canvas->translate(150, 0);
        canvas->save();
            canvas->rotate(30, 100, 100);
            canvas->drawRect(r, paint);
        canvas->restore();

        paint.setAntiAlias(true);
        canvas->translate(150, 0);
        canvas->save();
            canvas->rotate(30, 100, 100);
            canvas->drawRect(r, paint);
        canvas->restore();

        canvas->restore();
        canvas->translate(0, 150);
    }
}
