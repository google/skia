/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "gm.h"

#include "SkArithmeticImageFilter.h"
#include "SkBlurImageFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkImage.h"
#include "SkImageSource.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkXfermodeImageFilter.h"

class ImageFiltersGraphGM : public skiagm::GM {
public:
    ImageFiltersGraphGM() {}

protected:

    SkString onShortName() override {
        return SkString("imagefiltersgraph");
    }

    SkISize onISize() override { return SkISize::Make(600, 150); }

    void onOnceBeforeDraw() override {
        fImage = SkImage::MakeFromBitmap(
                ToolUtils::create_string_bitmap(100, 100, SK_ColorWHITE, 20, 70, 96, "e"));
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);
        {
            sk_sp<SkImageFilter> bitmapSource(SkImageSource::Make(fImage));
            sk_sp<SkColorFilter> cf(SkColorFilters::Blend(SK_ColorRED,
                                                                  SkBlendMode::kSrcIn));
            sk_sp<SkImageFilter> blur(SkBlurImageFilter::Make(4.0f, 4.0f, std::move(bitmapSource)));
            sk_sp<SkImageFilter> erode(SkErodeImageFilter::Make(4, 4, blur));
            sk_sp<SkImageFilter> color(SkColorFilterImageFilter::Make(std::move(cf),
                                                                      std::move(erode)));
            sk_sp<SkImageFilter> merge(SkMergeImageFilter::Make(blur, color));

            SkPaint paint;
            paint.setImageFilter(std::move(merge));
            canvas->drawPaint(paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            sk_sp<SkImageFilter> morph(SkDilateImageFilter::Make(5, 5, nullptr));

            SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                    0, SK_Scalar1, 0, 0, 0,
                                    0, 0, SK_Scalar1, 0, 0,
                                    0, 0, 0, 0.5f, 0 };

            sk_sp<SkColorFilter> matrixFilter(SkColorFilters::MatrixRowMajor255(matrix));
            sk_sp<SkImageFilter> colorMorph(SkColorFilterImageFilter::Make(std::move(matrixFilter),
                                                                           std::move(morph)));
            SkPaint paint;
            paint.setImageFilter(SkXfermodeImageFilter::Make(SkBlendMode::kSrcOver,
                                                             std::move(colorMorph)));

            DrawClippedImage(canvas, fImage.get(), paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            SkScalar matrix[20] = { SK_Scalar1, 0, 0, 0, 0,
                                    0, SK_Scalar1, 0, 0, 0,
                                    0, 0, SK_Scalar1, 0, 0,
                                    0, 0, 0, 0.5f, 0 };
            sk_sp<SkColorFilter> matrixCF(SkColorFilters::MatrixRowMajor255(matrix));
            sk_sp<SkImageFilter> matrixFilter(SkColorFilterImageFilter::Make(std::move(matrixCF),
                                                                             nullptr));
            sk_sp<SkImageFilter> offsetFilter(SkOffsetImageFilter::Make(10.0f, 10.f,
                                                                        matrixFilter));

            SkPaint paint;
            paint.setImageFilter(SkArithmeticImageFilter::Make(
                    0, 1, 1, 0, true, std::move(matrixFilter), std::move(offsetFilter), nullptr));

            DrawClippedImage(canvas, fImage.get(), paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            sk_sp<SkImageFilter> blur(SkBlurImageFilter::Make(SkIntToScalar(10),
                                                              SkIntToScalar(10),
                                                              nullptr));

            SkImageFilter::CropRect cropRect(SkRect::MakeWH(SkIntToScalar(95), SkIntToScalar(100)));
            SkPaint paint;
            paint.setImageFilter(
                SkXfermodeImageFilter::Make(SkBlendMode::kSrcIn, std::move(blur), nullptr,
                                            &cropRect));
            DrawClippedImage(canvas, fImage.get(), paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Dilate -> matrix convolution.
            // This tests that a filter using asFragmentProcessor (matrix
            // convolution) correctly handles a non-zero source offset
            // (supplied by the dilate).
            sk_sp<SkImageFilter> dilate(SkDilateImageFilter::Make(5, 5, nullptr));

            SkScalar kernel[9] = {
                SkIntToScalar(-1), SkIntToScalar( -1 ), SkIntToScalar(-1),
                SkIntToScalar(-1), SkIntToScalar(  7 ), SkIntToScalar(-1),
                SkIntToScalar(-1), SkIntToScalar( -1 ), SkIntToScalar(-1),
            };
            SkISize kernelSize = SkISize::Make(3, 3);
            SkScalar gain = 1.0f, bias = SkIntToScalar(0);
            SkIPoint kernelOffset = SkIPoint::Make(1, 1);
            auto tileMode = SkMatrixConvolutionImageFilter::kClamp_TileMode;
            bool convolveAlpha = false;
            sk_sp<SkImageFilter> convolve(SkMatrixConvolutionImageFilter::Make(kernelSize,
                                                                               kernel,
                                                                               gain,
                                                                               bias,
                                                                               kernelOffset,
                                                                               tileMode,
                                                                               convolveAlpha,
                                                                               std::move(dilate)));

            SkPaint paint;
            paint.setImageFilter(std::move(convolve));
            DrawClippedImage(canvas, fImage.get(), paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
        {
            // Test that crop offsets are absolute, not relative to the parent's crop rect.
            sk_sp<SkColorFilter> cf1(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kSrcIn));
            sk_sp<SkColorFilter> cf2(SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrcIn));
            SkImageFilter::CropRect outerRect(SkRect::MakeXYWH(SkIntToScalar(10), SkIntToScalar(10),
                                                               SkIntToScalar(80), SkIntToScalar(80)));
            SkImageFilter::CropRect innerRect(SkRect::MakeXYWH(SkIntToScalar(20), SkIntToScalar(20),
                                                               SkIntToScalar(60), SkIntToScalar(60)));
            sk_sp<SkImageFilter> color1(SkColorFilterImageFilter::Make(std::move(cf1),
                                                                       nullptr,
                                                                       &outerRect));
            sk_sp<SkImageFilter> color2(SkColorFilterImageFilter::Make(std::move(cf2),
                                                                       std::move(color1),
                                                                       &innerRect));

            SkPaint paint;
            paint.setImageFilter(std::move(color2));
            paint.setColor(SK_ColorRED);
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 100, 100), paint);
            canvas->translate(SkIntToScalar(100), 0);
        }
    }

private:
    static void DrawClippedImage(SkCanvas* canvas, const SkImage* image, const SkPaint& paint) {
        canvas->save();
        canvas->clipRect(SkRect::MakeIWH(image->width(), image->height()));
        canvas->drawImage(image, 0, 0, &paint);
        canvas->restore();
    }

    sk_sp<SkImage> fImage;

    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFiltersGraphGM;)
