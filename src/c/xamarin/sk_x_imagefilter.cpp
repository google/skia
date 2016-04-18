/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageFilter.h"
#include "../../include/effects/SkAlphaThresholdFilter.h"
#include "../../include/effects/SkBlurImageFilter.h"
#include "../../include/effects/SkColorFilterImageFilter.h"
#include "../../include/effects/SkComposeImageFilter.h"
#include "../../include/effects/SkDisplacementMapEffect.h"
#include "../../include/effects/SkDropShadowImageFilter.h"
#include "../../include/effects/SkLightingImageFilter.h"
#include "../../include/effects/SkMagnifierImageFilter.h"
#include "../../include/effects/SkMatrixConvolutionImageFilter.h"
#include "../../include/effects/SkMergeImageFilter.h"
#include "../../include/effects/SkMorphologyImageFilter.h"
#include "../../include/effects/SkOffsetImageFilter.h"
#include "../../include/effects/SkPictureImageFilter.h"
#include "../../include/effects/SkTestImageFilters.h"
#include "../../include/effects/SkTileImageFilter.h"
#include "../../include/effects/SkXfermodeImageFilter.h"

#include "xamarin/sk_x_imagefilter.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

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
    if (rect) {
        *rect = ToRect(AsImageFilterCropRect(cropRect)->rect());
    }
}

uint32_t sk_imagefilter_croprect_get_flags(sk_imagefilter_croprect_t* cropRect) {
    return AsImageFilterCropRect(cropRect)->flags();
}

void sk_imagefilter_unref(sk_imagefilter_t* cfilter) {
    SkSafeUnref(AsImageFilter(cfilter));
}

sk_imagefilter_t* sk_imagefilter_new_matrix(
    const sk_matrix_t* cmatrix,
    sk_filter_quality_t cquality,
    sk_imagefilter_t* input /*NULL*/) {

    SkMatrix matrix;
    from_c(cmatrix, &matrix);

    SkFilterQuality quality;
    if (!find_sk(cquality, &quality)) {
        return NULL;
    }

    SkImageFilter* filter = SkImageFilter::CreateMatrixFilter(matrix, quality, AsImageFilter(input));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_alpha_threshold(
    const sk_irect_t* region,
    float innerThreshold,
    float outerThreshold,
    sk_imagefilter_t* input /*NULL*/) {

    SkRegion r = SkRegion(AsIRect(*region));

    SkImageFilter* filter = SkAlphaThresholdFilter::Create(r, innerThreshold, outerThreshold, AsImageFilter(input));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_blur(
    float sigmaX,
    float sigmaY,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkBlurImageFilter::Create(sigmaX, sigmaY, AsImageFilter(input), AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_color_filter(
    sk_colorfilter_t* cf,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkColorFilterImageFilter::Create(AsColorFilter(cf), AsImageFilter(input), AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_compose(
    sk_imagefilter_t* outer,
    sk_imagefilter_t* inner) {

    SkImageFilter* filter = SkComposeImageFilter::Create(AsImageFilter(outer), AsImageFilter(inner));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_displacement_map_effect(
    sk_displacement_map_effect_channel_selector_type_t xChannelSelector,
    sk_displacement_map_effect_channel_selector_type_t yChannelSelector,
    float scale,
    sk_imagefilter_t* displacement,
    sk_imagefilter_t* color /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkDisplacementMapEffect::ChannelSelectorType xSel;
    if (!find_sk(xChannelSelector, &xSel)) {
        return NULL;
    }
    SkDisplacementMapEffect::ChannelSelectorType ySel;
    if (!find_sk(yChannelSelector, &ySel)) {
        return NULL;
    }

    SkImageFilter* filter = SkDisplacementMapEffect::Create(
        xSel,
        ySel, 
        scale, 
        AsImageFilter(displacement),
        AsImageFilter(color), 
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_downsample(
    float scale,
    sk_imagefilter_t* input /*NULL*/) {

    SkImageFilter* filter = SkDownSampleImageFilter::Create(
        scale,
        AsImageFilter(input));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_drop_shadow(
    float dx,
    float dy,
    float sigmaX,
    float sigmaY,
    sk_color_t color,
    sk_drop_shadow_image_filter_shadow_mode_t cShadowMode,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkDropShadowImageFilter::ShadowMode shadowMode;
    if (!find_sk(cShadowMode, &shadowMode)) {
        return NULL;
    }

    SkImageFilter* filter = SkDropShadowImageFilter::Create(
        dx,
        dy, 
        sigmaX, 
        sigmaY,
        color,
        shadowMode,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_distant_lit_diffuse(
    const sk_point3_t* direction,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkLightingImageFilter::CreateDistantLitDiffuse(
        *AsPoint3(direction),
        lightColor,
        surfaceScale,
        kd,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_point_lit_diffuse(
    const sk_point3_t* location,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkLightingImageFilter::CreatePointLitDiffuse(
        *AsPoint3(location),
        lightColor,
        surfaceScale,
        kd,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_spot_lit_diffuse(
    const sk_point3_t* location,
    const sk_point3_t* target,
    float specularExponent,
    float cutoffAngle,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkLightingImageFilter::CreateSpotLitDiffuse(
        *AsPoint3(location),
        *AsPoint3(target),
        specularExponent,
        cutoffAngle,
        lightColor,
        surfaceScale,
        kd,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_distant_lit_specular(
    const sk_point3_t* direction,
    sk_color_t lightColor,
    float surfaceScale,
    float ks,
    float shininess,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkLightingImageFilter::CreateDistantLitSpecular(
        *AsPoint3(direction),
        lightColor,
        surfaceScale,
        ks,
        shininess,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_point_lit_specular(
    const sk_point3_t* location,
    sk_color_t lightColor,
    float surfaceScale,
    float ks,
    float shininess,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkLightingImageFilter::CreatePointLitSpecular(
        *AsPoint3(location),
        lightColor,
        surfaceScale,
        ks,
        shininess,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
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
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkLightingImageFilter::CreateSpotLitSpecular(
        *AsPoint3(location),
        *AsPoint3(target),
        specularExponent,
        cutoffAngle,
        lightColor,
        surfaceScale,
        ks,
        shininess,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_magnifier(
    const sk_rect_t* src,
    float inset,
    sk_imagefilter_t* input /*NULL*/) {

    SkImageFilter* filter = SkMagnifierImageFilter::Create(
        *AsRect(src),
        inset,
        AsImageFilter(input));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_matrix_convolution(
    const sk_isize_t* kernelSize,
    const float kernel[],
    float gain,
    float bias,
    const sk_ipoint_t* kernelOffset,
    sk_matrix_convolution_tilemode_t ctileMode,
    bool convolveAlpha,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkMatrixConvolutionImageFilter::TileMode tileMode;
    if (!find_sk(ctileMode, &tileMode)) {
        return NULL;
    }

    SkImageFilter* filter = SkMatrixConvolutionImageFilter::Create(
        *AsISize(kernelSize),
        kernel,
        gain,
        bias,
        *AsIPoint(kernelOffset),
        tileMode,
        convolveAlpha,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_merge(
    sk_imagefilter_t* filters[],
    int count,
    const sk_xfermode_mode_t cmodes[] /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkXfermode::Mode* modes = new SkXfermode::Mode[count];
    for (int i = 0; i < count; i++) {
        if (!find_sk(cmodes[i], &modes[i])) {
            delete[] modes;
            return NULL;
        }
    }
    
    SkImageFilter* filter = SkMergeImageFilter::Create(
        AsImageFilters(filters),
        count,
        modes,
        AsImageFilterCropRect(cropRect));

    delete[] modes;

    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_dilate(
    int radiusX,
    int radiusY,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkDilateImageFilter::Create(
        radiusX,
        radiusY,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_erode(
    int radiusX,
    int radiusY,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkErodeImageFilter::Create(
        radiusX,
        radiusY,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_offset(
    float dx,
    float dy,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkImageFilter* filter = SkOffsetImageFilter::Create(
        dx,
        dy,
        AsImageFilter(input),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_picture(
    sk_picture_t* picture) {

    SkImageFilter* filter = SkPictureImageFilter::Create(
        AsPicture(picture));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_picture_with_croprect(
    sk_picture_t* picture,
    const sk_rect_t* cropRect) {

    SkImageFilter* filter = SkPictureImageFilter::Create(
        AsPicture(picture),
        *AsRect(cropRect));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_picture_for_localspace(
    sk_picture_t* picture,
    const sk_rect_t* cropRect,
    sk_filter_quality_t filterQuality) {

    SkFilterQuality quality;
    if (!find_sk(filterQuality, &quality)) {
        return NULL;
    }

    SkImageFilter* filter = SkPictureImageFilter::CreateForLocalSpace(
        AsPicture(picture),
        *AsRect(cropRect),
        quality);
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_tile(
    const sk_rect_t* src,
    const sk_rect_t* dst,
    sk_imagefilter_t* input) {

    SkImageFilter* filter = SkTileImageFilter::Create(
        *AsRect(src),
        *AsRect(dst),
        AsImageFilter(input));
    return ToImageFilter(filter);
}

sk_imagefilter_t* sk_imagefilter_new_xfermode(
    sk_xfermode_mode_t cmode,
    sk_imagefilter_t* background,
    sk_imagefilter_t* foreground /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/) {

    SkXfermode::Mode mode;
    if (!find_sk(cmode, &mode)) {
        return NULL;
    }

    SkImageFilter* filter = SkXfermodeImageFilter::Create(
        SkXfermode::Create(mode),
        AsImageFilter(background),
        AsImageFilter(foreground),
        AsImageFilterCropRect(cropRect));
    return ToImageFilter(filter);
}
