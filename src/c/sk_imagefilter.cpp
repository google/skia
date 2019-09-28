/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRegion.h"
#include "include/effects/SkAlphaThresholdFilter.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkComposeImageFilter.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkLightingImageFilter.h"
#include "include/effects/SkMagnifierImageFilter.h"
#include "include/effects/SkMatrixConvolutionImageFilter.h"
#include "include/effects/SkMergeImageFilter.h"
#include "include/effects/SkMorphologyImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"
#include "include/effects/SkPictureImageFilter.h"
#include "include/effects/SkTileImageFilter.h"
#include "include/effects/SkXfermodeImageFilter.h"
#include "include/effects/SkArithmeticImageFilter.h"
#include "include/effects/SkImageSource.h"
#include "include/effects/SkPaintImageFilter.h"

#include "include/c/sk_imagefilter.h"

#include "src/c/sk_types_priv.h"

sk_imagefilter_croprect_t* sk_imagefilter_croprect_new() {
    return (sk_imagefilter_croprect_t*) new SkImageFilter::CropRect();
}

sk_imagefilter_croprect_t* sk_imagefilter_croprect_new_with_rect(const sk_rect_t* rect, uint32_t flags) {
    return (sk_imagefilter_croprect_t*) new SkImageFilter::CropRect(*AsRect(rect), flags);
}

void sk_imagefilter_croprect_destructor(sk_imagefilter_croprect_t* cropRect) {
    delete AsImageFilterCropRect(cropRect);
}

void sk_imagefilter_croprect_get_rect(sk_imagefilter_croprect_t* cropRect, sk_rect_t* rect) {
    *rect = ToRect(AsImageFilterCropRect(cropRect)->rect());
}

uint32_t sk_imagefilter_croprect_get_flags(sk_imagefilter_croprect_t* cropRect) {
    return AsImageFilterCropRect(cropRect)->flags();
}

void sk_imagefilter_unref(sk_imagefilter_t* cfilter) {
    SkSafeUnref(AsImageFilter(cfilter));
}

sk_imagefilter_t* sk_imagefilter_new_matrix(const sk_matrix_t* cmatrix, sk_filter_quality_t cquality, sk_imagefilter_t* input) {
    return ToImageFilter(SkImageFilter::MakeMatrixFilter(AsMatrix(cmatrix), (SkFilterQuality)cquality, sk_ref_sp(AsImageFilter(input))).release());
}

sk_imagefilter_t* sk_imagefilter_new_alpha_threshold(const sk_region_t* region, float innerThreshold, float outerThreshold, sk_imagefilter_t* input) {
    return ToImageFilter(SkAlphaThresholdFilter::Make(*AsRegion(region), innerThreshold, outerThreshold, sk_ref_sp(AsImageFilter(input))).release());
}

sk_imagefilter_t* sk_imagefilter_new_blur(float sigmaX, float sigmaY, sk_imagefilter_t* input, const sk_imagefilter_croprect_t* cropRect) {
    return ToImageFilter(SkBlurImageFilter::Make(sigmaX, sigmaY, sk_ref_sp(AsImageFilter(input)), AsImageFilterCropRect(cropRect)).release());
}

sk_imagefilter_t* sk_imagefilter_new_color_filter(sk_colorfilter_t* cf, sk_imagefilter_t* input, const sk_imagefilter_croprect_t* cropRect) {
    return ToImageFilter(SkColorFilterImageFilter::Make(sk_ref_sp(AsColorFilter(cf)), sk_ref_sp(AsImageFilter(input)), AsImageFilterCropRect(cropRect)).release());
}

sk_imagefilter_t* sk_imagefilter_new_compose(sk_imagefilter_t* outer, sk_imagefilter_t* inner) {
    return ToImageFilter(SkComposeImageFilter::Make(sk_ref_sp(AsImageFilter(outer)), sk_ref_sp(AsImageFilter(inner))).release());
}

sk_imagefilter_t* sk_imagefilter_new_displacement_map_effect(sk_displacement_map_effect_channel_selector_type_t xChannelSelector, sk_displacement_map_effect_channel_selector_type_t yChannelSelector, float scale, sk_imagefilter_t* displacement, sk_imagefilter_t* color, const sk_imagefilter_croprect_t* cropRect) {
    sk_sp<SkImageFilter> filter = SkDisplacementMapEffect::Make(
        (SkDisplacementMapEffect::ChannelSelectorType)xChannelSelector,
        (SkDisplacementMapEffect::ChannelSelectorType)yChannelSelector, 
        scale, 
        sk_ref_sp(AsImageFilter(displacement)),
        sk_ref_sp(AsImageFilter(color)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_drop_shadow(float dx, float dy, float sigmaX, float sigmaY, sk_color_t color, sk_drop_shadow_image_filter_shadow_mode_t cShadowMode, sk_imagefilter_t* input, const sk_imagefilter_croprect_t* cropRect) {
    sk_sp<SkImageFilter> filter = SkDropShadowImageFilter::Make(
        dx,
        dy, 
        sigmaX, 
        sigmaY,
        color,
        (SkDropShadowImageFilter::ShadowMode)cShadowMode,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_distant_lit_diffuse(
    const sk_point3_t* direction,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakeDistantLitDiffuse(
        *AsPoint3(direction),
        lightColor,
        surfaceScale,
        kd,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_point_lit_diffuse(
    const sk_point3_t* location,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakePointLitDiffuse(
        *AsPoint3(location),
        lightColor,
        surfaceScale,
        kd,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_spot_lit_diffuse(
    const sk_point3_t* location,
    const sk_point3_t* target,
    float specularExponent,
    float cutoffAngle,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakeSpotLitDiffuse(
        *AsPoint3(location),
        *AsPoint3(target),
        specularExponent,
        cutoffAngle,
        lightColor,
        surfaceScale,
        kd,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_distant_lit_specular(
    const sk_point3_t* direction,
    sk_color_t lightColor,
    float surfaceScale,
    float ks,
    float shininess,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakeDistantLitSpecular(
        *AsPoint3(direction),
        lightColor,
        surfaceScale,
        ks,
        shininess,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_point_lit_specular(
    const sk_point3_t* location,
    sk_color_t lightColor,
    float surfaceScale,
    float ks,
    float shininess,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakePointLitSpecular(
        *AsPoint3(location),
        lightColor,
        surfaceScale,
        ks,
        shininess,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_spot_lit_specular(
    const sk_point3_t* location,
    const sk_point3_t* target,
    float specularExponent,
    float cutoffAngle,
    sk_color_t lightColor,
    float surfaceScale,
    float ks,
    float shininess,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakeSpotLitSpecular(
        *AsPoint3(location),
        *AsPoint3(target),
        specularExponent,
        cutoffAngle,
        lightColor,
        surfaceScale,
        ks,
        shininess,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_magnifier(
    const sk_rect_t* src,
    float inset,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkMagnifierImageFilter::Make(
        *AsRect(src),
        inset,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_matrix_convolution(
    const sk_isize_t* kernelSize,
    const float kernel[],
    float gain,
    float bias,
    const sk_ipoint_t* kernelOffset,
    sk_matrix_convolution_tilemode_t ctileMode,
    bool convolveAlpha,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkMatrixConvolutionImageFilter::Make(
        *AsISize(kernelSize),
        kernel,
        gain,
        bias,
        *AsIPoint(kernelOffset),
        (SkMatrixConvolutionImageFilter::TileMode)ctileMode,
        convolveAlpha,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_merge(
    sk_imagefilter_t* cfilters[],
    int count,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter>* filters = new sk_sp<SkImageFilter>[count];
    for (int i = 0; i < count; i++) {
        filters[i] = sk_ref_sp(AsImageFilter(cfilters[i]));
    }
    
    sk_sp<SkImageFilter> filter = SkMergeImageFilter::Make(
        filters,
        count,
        AsImageFilterCropRect(cropRect));

    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_dilate(
    int radiusX,
    int radiusY,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkDilateImageFilter::Make(
        radiusX,
        radiusY,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_erode(
    int radiusX,
    int radiusY,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkErodeImageFilter::Make(
        radiusX,
        radiusY,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_offset(
    float dx,
    float dy,
    sk_imagefilter_t* input,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkOffsetImageFilter::Make(
        dx,
        dy,
        sk_ref_sp(AsImageFilter(input)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_picture(
    sk_picture_t* picture) {

    sk_sp<SkImageFilter> filter = SkPictureImageFilter::Make(
        sk_ref_sp(AsPicture(picture)));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_picture_with_croprect(
    sk_picture_t* picture,
    const sk_rect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkPictureImageFilter::Make(
        sk_ref_sp(AsPicture(picture)),
        *AsRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_tile(
    const sk_rect_t* src,
    const sk_rect_t* dst,
    sk_imagefilter_t* input) {

    sk_sp<SkImageFilter> filter = SkTileImageFilter::Make(
        *AsRect(src),
        *AsRect(dst),
        sk_ref_sp(AsImageFilter(input)));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_xfermode(
    sk_blendmode_t cmode,
    sk_imagefilter_t* background,
    sk_imagefilter_t* foreground,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkXfermodeImageFilter::Make(
        (SkBlendMode)cmode,
        sk_ref_sp(AsImageFilter(background)),
        sk_ref_sp(AsImageFilter(foreground)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

SK_API sk_imagefilter_t* sk_imagefilter_new_arithmetic(
    float k1, float k2, float k3, float k4,
    bool enforcePMColor,
    sk_imagefilter_t* background,
    sk_imagefilter_t* foreground,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkArithmeticImageFilter::Make(
        k1, k2, k3, k4,
        enforcePMColor,
        sk_ref_sp(AsImageFilter(background)),
        sk_ref_sp(AsImageFilter(foreground)),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_image_source(
    sk_image_t* image,
    const sk_rect_t* srcRect,
    const sk_rect_t* dstRect,
    sk_filter_quality_t filterQuality) {

    sk_sp<SkImageFilter> filter = SkImageSource::Make(
        sk_ref_sp(AsImage(image)),
        *AsRect(srcRect),
        *AsRect(dstRect),
        (SkFilterQuality)filterQuality);
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_image_source_default(
    sk_image_t* image) {

    sk_sp<SkImageFilter> filter = SkImageSource::Make(
        sk_ref_sp(AsImage(image)));
    return ToImageFilter(filter.release());
}

sk_imagefilter_t* sk_imagefilter_new_paint(
    const sk_paint_t* paint,
    const sk_imagefilter_croprect_t* cropRect) {

    sk_sp<SkImageFilter> filter = SkPaintImageFilter::Make(
        *AsPaint(paint),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter.release());
}
