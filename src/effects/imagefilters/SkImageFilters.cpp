/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

// TODO (michaelludwig) - Right now there is a bit of a weird dependency where the implementations
// of the new, preferred filter factories depends on the per-filter headers in include/effects,
// which have themselves been marked as deprecated. But, once clients are updated to use the
// new factories implemented in this file, the per-filter headers can go into
// src/effects/imagefilters and will no longer be "deprecated" since they've been made fully
// internal at that point.
#include "include/effects/SkAlphaThresholdFilter.h"
#include "include/effects/SkArithmeticImageFilter.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkComposeImageFilter.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkImageSource.h"
#include "include/effects/SkLightingImageFilter.h"
#include "include/effects/SkMagnifierImageFilter.h"
#include "include/effects/SkMatrixConvolutionImageFilter.h"
#include "include/effects/SkMergeImageFilter.h"
#include "include/effects/SkMorphologyImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"
#include "include/effects/SkPaintImageFilter.h"
#include "include/effects/SkPictureImageFilter.h"
#include "include/effects/SkTileImageFilter.h"
#include "include/effects/SkXfermodeImageFilter.h"

// TODO (michaelludwig) - Once SkCanvas can draw the results of a filter with any transform, this
// filter can be moved out of core
#include "src/core/SkMatrixImageFilter.h"

// TODO (michaelludwig) - We are phasing out the use of SkImageFilter::CropRect since it does not
// appear as though edge flags are actually used and will move towards an explicit cropping filter.
// To assist with this, the new factory functions just take the basic SkIRect* even though the
// implementations have not been updated yet.
static SkImageFilter::CropRect make_crop_rect(const SkIRect* cropRect) {
    return cropRect ? SkImageFilter::CropRect(SkRect::Make(*cropRect))
                    : SkImageFilter::CropRect(SkRect::MakeEmpty(), 0x0);
}

void SkImageFilters::RegisterFlattenables() {
    SkAlphaThresholdFilter::RegisterFlattenables();
    SkArithmeticImageFilter::RegisterFlattenables();
    SkBlurImageFilter::RegisterFlattenables();
    SkColorFilterImageFilter::RegisterFlattenables();
    SkComposeImageFilter::RegisterFlattenables();
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
    SkXfermodeImageFilter::RegisterFlattenables();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::AlphaThreshold(
        const SkRegion& region, SkScalar innerMin, SkScalar outerMax, sk_sp<SkImageFilter> input,
        const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkAlphaThresholdFilter::Make(region, innerMin, outerMax, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::Arithmetic(
        SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4, bool enforcePMColor,
        sk_sp<SkImageFilter> background, sk_sp<SkImageFilter> foreground, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkArithmeticImageFilter::Make(k1, k2, k3, k4, enforcePMColor, std::move(background),
                                         std::move(foreground), &r);
}

sk_sp<SkImageFilter> SkImageFilters::Blur(
        SkScalar sigmaX, SkScalar sigmaY, SkTileMode tileMode, sk_sp<SkImageFilter> input,
        const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkBlurImageFilter::Make(sigmaX, sigmaY, tileMode, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::ColorFilter(
        sk_sp<SkColorFilter> cf, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkColorFilterImageFilter::Make(std::move(cf), std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::Compose(
        sk_sp<SkImageFilter> outer, sk_sp<SkImageFilter> inner) {
    return SkComposeImageFilter::Make(std::move(outer), std::move(inner));
}

sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkDisplacementMapEffect::Make(xChannelSelector, yChannelSelector, scale,
                                         std::move(displacement), std::move(color), &r);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadow(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    // TODO (michaelludwig) - Once SkDropShadowImageFilter is fully hidden, this can be updated to
    // pass a constant bool into the internal factory.
    return SkDropShadowImageFilter::Make(
            dx, dy, sigmaX, sigmaY, color,
            SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
            std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadowOnly(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    // TODO (michaelludwig) - Once SkDropShadowImageFilter is fully hidden, this can be updated to
    // pass a constant bool into the internal factory.
    return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color,
                                         SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode,
                                         std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::Image(
        sk_sp<SkImage> image, const SkRect& srcRect, const SkRect& dstRect,
        SkFilterQuality filterQuality) {
    return SkImageSource::Make(std::move(image), srcRect, dstRect, filterQuality);
}

sk_sp<SkImageFilter> SkImageFilters::Magnifier(
        const SkRect& srcRect, SkScalar inset, sk_sp<SkImageFilter> input,const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkMagnifierImageFilter::Make(srcRect, inset, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::MatrixConvolution(
        const SkISize& kernelSize,  const SkScalar kernel[], SkScalar gain, SkScalar bias,
        const SkIPoint& kernelOffset, SkTileMode tileMode, bool convolveAlpha,
        sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkMatrixConvolutionImageFilter::Make(kernelSize, kernel, gain, bias, kernelOffset,
                                                tileMode, convolveAlpha, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, SkFilterQuality filterQuality, sk_sp<SkImageFilter> input) {
    return SkMatrixImageFilter::Make(transform, filterQuality, std::move(input));
}

sk_sp<SkImageFilter> SkImageFilters::Merge(
        sk_sp<SkImageFilter>* const filters, int count, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkMergeImageFilter::Make(filters, count, &r);
}

sk_sp<SkImageFilter> SkImageFilters::Offset(
        SkScalar dx, SkScalar dy, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkOffsetImageFilter::Make(dx, dy, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::Paint(const SkPaint& paint, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkPaintImageFilter::Make(paint, &r);
}

sk_sp<SkImageFilter> SkImageFilters::Picture(sk_sp<SkPicture> pic, const SkRect& targetRect) {
    return SkPictureImageFilter::Make(std::move(pic), targetRect);
}

sk_sp<SkImageFilter> SkImageFilters::Tile(
        const SkRect& src, const SkRect& dst, sk_sp<SkImageFilter> input) {
    return SkTileImageFilter::Make(src, dst, std::move(input));
}

sk_sp<SkImageFilter> SkImageFilters::Xfermode(
        SkBlendMode mode, sk_sp<SkImageFilter> background, sk_sp<SkImageFilter> foreground,
        const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkXfermodeImageFilter::Make(mode, std::move(background), std::move(foreground), &r);
}

// Morphology filter effects

sk_sp<SkImageFilter> SkImageFilters::Dilate(
        int radiusX, int radiusY, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkDilateImageFilter::Make(radiusX, radiusY, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::Erode(
        int radiusX, int radiusY, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkErodeImageFilter::Make(radiusX, radiusY, std::move(input), &r);
}

// Lighting filter effects

sk_sp<SkImageFilter> SkImageFilters::DistantLitDiffuse(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkLightingImageFilter::MakeDistantLitDiffuse(direction, lightColor, surfaceScale, kd,
                                                        std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitDiffuse(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkLightingImageFilter::MakePointLitDiffuse(location, lightColor, surfaceScale, kd,
                                                      std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitDiffuse(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar kd,
        sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkLightingImageFilter::MakeSpotLitDiffuse(location, target, falloffExponent, cutoffAngle,
                                                     lightColor, surfaceScale, kd,
                                                     std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::DistantLitSpecular(
        const SkPoint3& direction, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkLightingImageFilter::MakeDistantLitSpecular(direction, lightColor, surfaceScale,
                                                         ks, shininess, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::PointLitSpecular(
        const SkPoint3& location, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkLightingImageFilter::MakePointLitSpecular(location, lightColor, surfaceScale, ks,
                                                       shininess, std::move(input), &r);
}

sk_sp<SkImageFilter> SkImageFilters::SpotLitSpecular(
        const SkPoint3& location, const SkPoint3& target, SkScalar falloffExponent,
        SkScalar cutoffAngle, SkColor lightColor, SkScalar surfaceScale, SkScalar ks,
        SkScalar shininess, sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    SkImageFilter::CropRect r = make_crop_rect(cropRect);
    return SkLightingImageFilter::MakeSpotLitSpecular(location, target, falloffExponent,
                                                      cutoffAngle, lightColor, surfaceScale,
                                                      ks, shininess, std::move(input), &r);
}
