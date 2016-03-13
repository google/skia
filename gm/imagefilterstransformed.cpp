/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SkBlurImageFilter.h"
#include "SkColor.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkMorphologyImageFilter.h"
#include "SkScalar.h"
#include "SkSurface.h"
#include "gm.h"

namespace skiagm {

// This GM draws image filters with a CTM containing shearing / rotation.
// It checks that the scale portion of the CTM is correctly extracted
// and applied to the image inputs separately from the non-scale portion.

static SkImage* make_gradient_circle(int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = SkMinScalar(x, y) * 0.8f;

    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(width, height));
    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(0x00000000);
    SkColor colors[2];
    colors[0] = SK_ColorWHITE;
    colors[1] = SK_ColorBLACK;
    SkAutoTUnref<SkShader> shader(
        SkGradientShader::CreateRadial(SkPoint::Make(x, y), radius, colors, nullptr, 2,
                                       SkShader::kClamp_TileMode)
    );
    SkPaint paint;
    paint.setShader(shader);
    canvas->drawCircle(x, y, radius, paint);

    return surface->newImageSnapshot();
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
        fCheckerboard.reset(SkImage::NewFromBitmap(
            sk_tool_utils::create_checkerboard_bitmap(64, 64, 0xFFA0A0A0, 0xFF404040, 8)));
        fGradientCircle.reset(make_gradient_circle(64, 64));
    }

    void onDraw(SkCanvas* canvas) override {
        SkAutoTUnref<SkImageFilter> gradient(SkImageSource::Create(fGradientCircle));
        SkAutoTUnref<SkImageFilter> checkerboard(SkImageSource::Create(fCheckerboard));
        SkImageFilter* filters[] = {
            SkBlurImageFilter::Create(12, 0),
            SkDropShadowImageFilter::Create(0, 15, 8, 0, SK_ColorGREEN,
                SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode),
            SkDisplacementMapEffect::Create(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                            SkDisplacementMapEffect::kR_ChannelSelectorType,
                                            12,
                                            gradient.get(),
                                            checkerboard.get()),
            SkDilateImageFilter::Create(2, 2, checkerboard.get()),
            SkErodeImageFilter::Create(2, 2, checkerboard.get()),
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

        for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
            SkSafeUnref(filters[i]);
        }
    }

private:
    SkAutoTUnref<SkImage> fCheckerboard;
    SkAutoTUnref<SkImage> fGradientCircle;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ImageFiltersTransformedGM; )

}
