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
#include "include/core/SkFont.h"
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
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/timer/TimeUtils.h"

#include <utility>

namespace skiagm {

// This GM draws image filters with a CTM containing shearing / rotation.
// It checks that the scale portion of the CTM is correctly extracted
// and applied to the image inputs separately from the non-scale portion.

static sk_sp<SkImage> make_gradient_circle(int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = std::min(x, y) * 0.8f;

    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height)));
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
    SkString getName() const override { return SkString("imagefilterstransformed"); }

    SkISize getISize() override { return SkISize::Make(420, 240); }

    void onOnceBeforeDraw() override {
        fCheckerboard =
                ToolUtils::create_checkerboard_image(64, 64, 0xFFA0A0A0, 0xFF404040, 8);
        fGradientCircle = make_gradient_circle(64, 64);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkImageFilter> gradient(SkImageFilters::Image(fGradientCircle,
                                                            SkFilterMode::kLinear));
        sk_sp<SkImageFilter> checkerboard(SkImageFilters::Image(fCheckerboard,
                                                                SkFilterMode::kLinear));
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
            for (size_t i = 0; i < std::size(filters); ++i) {
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
    using INHERITED = GM;
};
DEF_GM( return new ImageFiltersTransformedGM; )
}  // namespace skiagm

//////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM(rotate_imagefilter, canvas, 500, 500) {
    SkPaint paint;

    const SkRect r = SkRect::MakeXYWH(50, 50, 100, 100);

    sk_sp<SkImageFilter> filters[] = {
        nullptr,
        SkImageFilters::Blur(6, 0, nullptr),
        SkImageFilters::Blend(SkBlendMode::kSrcOver, nullptr),
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

class ImageFilterMatrixWLocalMatrix : public skiagm::GM {
public:

    // Start at 132 degrees, since that resulted in a skipped draw before the fix to
    // SkLocalMatrixImageFilter's computeFastBounds() function.
    ImageFilterMatrixWLocalMatrix() : fDegrees(132.f) {}

protected:
    SkString getName() const override { return SkString("imagefilter_matrix_localmatrix"); }

    SkISize getISize() override { return SkISize::Make(512, 512); }

    bool onAnimate(double nanos) override {
        // Animate the rotation angle to ensure the local matrix bounds modifications work
        // for a variety of transformations.
        fDegrees = TimeUtils::Scaled(1e-9f * nanos, 360.f);
        return true;
    }

    void onOnceBeforeDraw() override {
        fImage = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    }

    void onDraw(SkCanvas* canvas) override {
        SkMatrix localMatrix;
        localMatrix.preTranslate(128, 128);
        localMatrix.preScale(2.0f, 2.0f);

        // This matrix applies a rotate around the center of the image (prior to the simulated
        // hi-dpi 2x device scale).
        SkMatrix filterMatrix;
        filterMatrix.setRotate(fDegrees, 64, 64);

        sk_sp<SkImageFilter> filter =
                SkImageFilters::MatrixTransform(filterMatrix,
                                                SkSamplingOptions(SkFilterMode::kLinear), nullptr)
                             ->makeWithLocalMatrix(localMatrix);

        SkPaint p;
        p.setImageFilter(filter);
        canvas->drawImage(fImage.get(), 128, 128, SkSamplingOptions(), &p);
    }

private:
    SkScalar fDegrees;
    sk_sp<SkImage> fImage;
};

DEF_GM(return new ImageFilterMatrixWLocalMatrix();)

class ImageFilterComposedTransform : public skiagm::GM {
public:

    // Start at 70 degrees since that highlighted the issue in skbug.com/10888
    ImageFilterComposedTransform() : fDegrees(70.f) {}

protected:
    SkString getName() const override { return SkString("imagefilter_composed_transform"); }

    SkISize getISize() override { return SkISize::Make(512, 512); }

    bool onAnimate(double nanos) override {
        // Animate the rotation angle to test a variety of transformations
        fDegrees = TimeUtils::Scaled(1e-9f * nanos, 360.f);
        return true;
    }

    void onOnceBeforeDraw() override {
        fImage = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    }

    void onDraw(SkCanvas* canvas) override {
        SkMatrix matrix = SkMatrix::RotateDeg(fDegrees);
        // All four quadrants should render the same
        this->drawFilter(canvas, 0.f, 0.f, this->makeDirectFilter(matrix));
        this->drawFilter(canvas, 256.f, 0.f, this->makeEarlyComposeFilter(matrix));
        this->drawFilter(canvas, 0.f, 256.f, this->makeLateComposeFilter(matrix));
        this->drawFilter(canvas, 256.f, 256.f, this->makeFullComposeFilter(matrix));
    }

private:
    SkScalar fDegrees;
    sk_sp<SkImage> fImage;

    void drawFilter(SkCanvas* canvas, SkScalar tx, SkScalar ty, sk_sp<SkImageFilter> filter) const {
        SkPaint p;
        p.setImageFilter(std::move(filter));

        canvas->save();
        canvas->translate(tx, ty);
        canvas->clipRect(SkRect::MakeWH(256, 256));
        canvas->scale(0.5f, 0.5f);
        canvas->translate(128, 128);
        canvas->drawImage(fImage, 0, 0, SkSamplingOptions(), &p);
        canvas->restore();
    }

    // offset(matrix(offset))
    sk_sp<SkImageFilter> makeDirectFilter(const SkMatrix& matrix) const {
        SkPoint v = {fImage->width() / 2.f, fImage->height() / 2.f};
        sk_sp<SkImageFilter> filter = SkImageFilters::Offset(-v.fX, -v.fY, nullptr);
        filter = SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(SkFilterMode::kLinear),
                                                 std::move(filter));
        filter = SkImageFilters::Offset(v.fX, v.fY, std::move(filter));
        return filter;
    }

    // offset(compose(matrix, offset))
    sk_sp<SkImageFilter> makeEarlyComposeFilter(const SkMatrix& matrix) const {
        SkPoint v = {fImage->width() / 2.f, fImage->height() / 2.f};
        sk_sp<SkImageFilter> offset = SkImageFilters::Offset(-v.fX, -v.fY, nullptr);
        sk_sp<SkImageFilter> filter = SkImageFilters::MatrixTransform(
                matrix, SkSamplingOptions(SkFilterMode::kLinear), nullptr);
        filter = SkImageFilters::Compose(std::move(filter), std::move(offset));
        filter = SkImageFilters::Offset(v.fX, v.fY, std::move(filter));
        return filter;
    }

    // compose(offset, matrix(offset))
    sk_sp<SkImageFilter> makeLateComposeFilter(const SkMatrix& matrix) const {
        SkPoint v = {fImage->width() / 2.f, fImage->height() / 2.f};
        sk_sp<SkImageFilter> filter = SkImageFilters::Offset(-v.fX, -v.fY, nullptr);
        filter = SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(SkFilterMode::kLinear),
                                                 std::move(filter));
        sk_sp<SkImageFilter> offset = SkImageFilters::Offset(v.fX, v.fY, nullptr);
        filter = SkImageFilters::Compose(std::move(offset), std::move(filter));
        return filter;
    }

    // compose(offset, compose(matrix, offset))
    sk_sp<SkImageFilter> makeFullComposeFilter(const SkMatrix& matrix) const {
        SkPoint v = {fImage->width() / 2.f, fImage->height() / 2.f};
        sk_sp<SkImageFilter> offset = SkImageFilters::Offset(-v.fX, -v.fY, nullptr);
        sk_sp<SkImageFilter> filter = SkImageFilters::MatrixTransform(
                matrix, SkSamplingOptions(SkFilterMode::kLinear), nullptr);
        filter = SkImageFilters::Compose(std::move(filter), std::move(offset));
        offset = SkImageFilters::Offset(v.fX, v.fY, nullptr);
        filter = SkImageFilters::Compose(std::move(offset), std::move(filter));
        return filter;
    }
};

DEF_GM(return new ImageFilterComposedTransform();)

// Tests SkImageFilters::Image under tricky matrices (mirrors and perspective)
DEF_SIMPLE_GM(imagefilter_transformed_image, canvas, 256, 256) {
    sk_sp<SkImage> image = ToolUtils::GetResourceAsImage("images/color_wheel.png");
    sk_sp<SkImageFilter> imageFilter = SkImageFilters::Image(image, SkFilterMode::kLinear);

    const SkRect imageRect = SkRect::MakeIWH(image->width(), image->height());

    SkM44 m1 = SkM44::Translate(0.9f * image->width(), 0.1f * image->height()) *
               SkM44::Scale(-.8f, .8f);

    SkM44 m2 = SkM44::RectToRect({-1.f, -1.f, 1.f, 1.f}, imageRect) *
               SkM44::Perspective(0.01f, 100.f, SK_ScalarPI / 3.f) *
               SkM44::Translate(0.f, 0.f, -2.f) *
               SkM44::Rotate({0.f, 1.f, 0.f}, SK_ScalarPI / 6.f) *
               SkM44::RectToRect(imageRect, {-1.f, -1.f, 1.f, 1.f});

    SkFont font = ToolUtils::DefaultPortableFont();
    canvas->drawString("Columns should match", 5.f, 15.f, font, SkPaint());
    canvas->translate(0.f, 10.f);

    SkSamplingOptions sampling(SkFilterMode::kLinear);
    for (auto m : {m1, m2}) {
        canvas->save();
        for (bool canvasTransform : {false, true}) {
            canvas->save();
            canvas->clipRect(imageRect);

            sk_sp<SkImageFilter> finalFilter;
            if (canvasTransform) {
                canvas->concat(m);
                finalFilter = imageFilter;
            } else {
                finalFilter = SkImageFilters::MatrixTransform(m.asM33(), sampling, imageFilter);
            }

            SkPaint paint;
            paint.setImageFilter(std::move(finalFilter));
            canvas->drawPaint(paint);

            canvas->restore();
            canvas->translate(image->width(), 0.f);
        }
        canvas->restore();

        canvas->translate(0.f, image->height());
    }
}
