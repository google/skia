/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkPaint.h"

#include "src/effects/imagefilters/SkDisplacementMapEffect.h"
#include "src/effects/imagefilters/SkDropShadowImageFilter.h"
#include "src/effects/imagefilters/SkImageSource.h"
#include "src/effects/imagefilters/SkLightingImageFilter.h"
#include "src/effects/imagefilters/SkMagnifierImageFilter.h"
#include "src/effects/imagefilters/SkMatrixConvolutionImageFilter.h"
#include "src/effects/imagefilters/SkMergeImageFilter.h"
#include "src/effects/imagefilters/SkMorphologyImageFilter.h"
#include "src/effects/imagefilters/SkOffsetImageFilter.h"
#include "src/effects/imagefilters/SkPaintImageFilter.h"
#include "src/effects/imagefilters/SkPictureImageFilter.h"
#include "src/effects/imagefilters/SkTileImageFilter.h"

// TODO (michaelludwig) - Once SkCanvas can draw the results of a filter with any transform, this
// filter can be moved out of core
#include "src/core/SkMatrixImageFilter.h"

void SkImageFilters::RegisterFlattenables() {
    SkDilateImageFilter::RegisterFlattenables();
    SkDisplacementMapEffect::RegisterFlattenables();
    SkDropShadowImageFilter::RegisterFlattenables();
    SkImageSource::RegisterFlattenables();
    SkLightingImageFilter::RegisterFlattenables();
    SkMagnifierImageFilter::RegisterFlattenables();
    SkMatrixConvolutionImageFilter::RegisterFlattenables();
    SkMergeImageFilter::RegisterFlattenables();
    SkOffsetImageFilter::RegisterFlattenables();
    SkPaintImageFilter::RegisterFlattenables();
    SkPictureImageFilter::RegisterFlattenables();
    SkTileImageFilter::RegisterFlattenables();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const CropRect& cropRect) {
    return SkDisplacementMapEffect::Make(xChannelSelector, yChannelSelector, scale,
                                         std::move(displacement), std::move(color), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadow(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    // TODO (michaelludwig) - Once SkDropShadowImageFilter is fully hidden, this can be updated to
    // pass a constant bool into the internal factory.
    return SkDropShadowImageFilter::Make(
            dx, dy, sigmaX, sigmaY, color,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
            std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadowOnly(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    // TODO (michaelludwig) - Once SkDropShadowImageFilter is fully hidden, this can be updated to
    // pass a constant bool into the internal factory.
    return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color,
                                         SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode,
                                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Image(
        sk_sp<SkImage> image, const SkRect& srcRect, const SkRect& dstRect,
        const SkSamplingOptions& sampling) {
    return SkImageSource::Make(std::move(image), srcRect, dstRect, sampling);
}

sk_sp<SkImageFilter> SkImageFilters::Image(sk_sp<SkImage> image,
                                           const SkSamplingOptions& sampling) {
    if (image) {
        auto r = SkRect::MakeIWH(image->width(), image->height());
        return Image(std::move(image), r, r, sampling);
    }
    return nullptr;
}

sk_sp<SkImageFilter> SkImageFilters::Magnifier(
        const SkRect& srcRect, SkScalar inset, sk_sp<SkImageFilter> input,
        const CropRect& cropRect) {
    return SkMagnifierImageFilter::Make(srcRect, inset, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::MatrixConvolution(
        const SkISize& kernelSize,  const SkScalar kernel[], SkScalar gain, SkScalar bias,
        const SkIPoint& kernelOffset, SkTileMode tileMode, bool convolveAlpha,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkMatrixConvolutionImageFilter::Make(kernelSize, kernel, gain, bias, kernelOffset,
                                                tileMode, convolveAlpha, std::move(input),
                                                cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, const SkSamplingOptions& sampling, sk_sp<SkImageFilter> input) {
    return SkMatrixImageFilter::Make(transform, sampling, std::move(input));
}

#ifdef SK_SUPPORT_LEGACY_MATRIX_IMAGEFILTER
sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, SkFilterQuality filterQuality, sk_sp<SkImageFilter> input) {
    auto sampling = SkSamplingOptions(filterQuality,
                                      SkSamplingOptions::kMedium_asMipmapLinear);
    return SkMatrixImageFilter::Make(transform, sampling, std::move(input));
}
#endif

sk_sp<SkImageFilter> SkImageFilters::Merge(
        sk_sp<SkImageFilter>* const filters, int count, const CropRect& cropRect) {
    return SkMergeImageFilter::Make(filters, count, cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Offset(
        SkScalar dx, SkScalar dy, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkOffsetImageFilter::Make(dx, dy, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Paint(const SkPaint& paint, const CropRect& cropRect) {
    return SkPaintImageFilter::Make(paint, cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Picture(sk_sp<SkPicture> pic, const SkRect& targetRect) {
    return SkPictureImageFilter::Make(std::move(pic), targetRect);
}

sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader, Dither dither,
                                            const CropRect& cropRect) {
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setDither((bool) dither);
    return SkPaintImageFilter::Make(paint, cropRect);
}

#ifdef SK_SUPPORT_LEGACY_IMPLICIT_FILTERQUALITY
sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader, Dither dither,
                                            SkFilterQuality filterQuality,
                                            const CropRect& cropRect) {
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setDither((bool) dither);
    // For SkImage::makeShader() shaders using SkImageShader::kInheritFromPaint sampling options
    paint.setFilterQuality(filterQuality);
    return SkPaintImageFilter::Make(paint, cropRect);
}
#endif

sk_sp<SkImageFilter> SkImageFilters::Tile(
        const SkRect& src, const SkRect& dst, sk_sp<SkImageFilter> input) {
    return SkTileImageFilter::Make(src, dst, std::move(input));
}

// Morphology filter effects

sk_sp<SkImageFilter> SkImageFilters::Dilate(
        SkScalar radiusX, SkScalar radiusY, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkDilateImageFilter::Make(radiusX, radiusY, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Erode(
        SkScalar radiusX, SkScalar radiusY, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkErodeImageFilter::Make(radiusX, radiusY, std::move(input), cropRect);
}

// Lighting filter effects

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkLightingImageFilter::MakeDistantLitDiffuse(direction, lightColor, surfaceScale, kd,
                                                        std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkLightingImageFilter::MakePointLitDiffuse(location, lightColor, surfaceScale, kd,
                                                      std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkLightingImageFilter::MakeSpotLitDiffuse(location, target, falloffExponent, cutoffAngle,
                                                     lightColor, surfaceScale, kd,
                                                     std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkLightingImageFilter::MakeDistantLitSpecular(direction, lightColor, surfaceScale,
                                                         ks, shininess, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkLightingImageFilter::MakePointLitSpecular(location, lightColor, surfaceScale, ks,
                                                       shininess, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkLightingImageFilter::MakeSpotLitSpecular(location, target, falloffExponent,
                                                      cutoffAngle, lightColor, surfaceScale,
                                                      ks, shininess, std::move(input), cropRect);
}
