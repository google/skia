/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_maskfilter_DEFINED
#define sk_maskfilter_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

///////////////////////////////////////////////////////////////////////////////////////////

/**
    Increment the reference count on the given sk_maskfilter_t. Must be
    balanced by a call to sk_maskfilter_unref().
*/
SK_API void sk_maskfilter_ref(sk_maskfilter_t*);
/**
    Decrement the reference count. If the reference count is 1 before
    the decrement, then release both the memory holding the
    sk_maskfilter_t and any other associated resources.  New
    sk_maskfilter_t are created with a reference count of 1.
*/
SK_API void sk_maskfilter_unref(sk_maskfilter_t*);

/**
    Create a blur maskfilter.
    @param sk_blurstyle_t The SkBlurStyle to use
    @param sigma Standard deviation of the Gaussian blur to apply. Must be > 0.
*/
SK_API sk_maskfilter_t* sk_maskfilter_new_blur(sk_blurstyle_t, float sigma);

SK_API sk_maskfilter_t* sk_maskfilter_new_emboss(
    float blurSigma,
    const float direction[3],
    float ambient, 
    float specular);

SK_API sk_maskfilter_t* sk_maskfilter_new_table(
    const uint8_t table[256]);

SK_API sk_maskfilter_t* sk_maskfilter_new_gamma(
    float gamma);

SK_API sk_maskfilter_t* sk_maskfilter_new_clip(
    uint8_t min,
    uint8_t max);

///////////////////////////////////////////////////////////////////////////////////////////

SK_API sk_imagefilter_croprect_t* sk_imagefilter_croprect_new();

SK_API sk_imagefilter_croprect_t* sk_imagefilter_croprect_new_with_rect(const sk_rect_t* rect, uint32_t flags);

SK_API void sk_imagefilter_croprect_destructor(sk_imagefilter_croprect_t* cropRect);

SK_API void sk_imagefilter_croprect_get_rect(sk_imagefilter_croprect_t* cropRect, sk_rect_t* rect);

SK_API uint32_t sk_imagefilter_croprect_get_flags(sk_imagefilter_croprect_t* cropRect);

///////////////////////////////////////////////////////////////////////////////////////////

SK_API void sk_imagefilter_unref(sk_imagefilter_t*);

SK_API sk_imagefilter_t* sk_imagefilter_new_matrix(
    const sk_matrix_t* matrix, 
    sk_filter_quality_t quality, 
    sk_imagefilter_t* input /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_alpha_threshold(
    const sk_irect_t* region,
    float innerThreshold,
    float outerThreshold, 
    sk_imagefilter_t* input /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_blur(
    float sigmaX,
    float sigmaY,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_color_filter(
    sk_colorfilter_t* cf,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_compose(
    sk_imagefilter_t* outer,
    sk_imagefilter_t* inner);

SK_API sk_imagefilter_t* sk_imagefilter_new_displacement_map_effect(
    sk_displacement_map_effect_channel_selector_type_t xChannelSelector,
    sk_displacement_map_effect_channel_selector_type_t yChannelSelector,
    float scale,
    sk_imagefilter_t* displacement,
    sk_imagefilter_t* color /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_downsample(
    float scale,
    sk_imagefilter_t* input /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_drop_shadow(
    float dx,
    float dy,
    float sigmaX,
    float sigmaY,
    sk_color_t color, 
    sk_drop_shadow_image_filter_shadow_mode_t shadowMode,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_distant_lit_diffuse(
    const sk_point3_t* direction,
    sk_color_t lightColor,
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_point_lit_diffuse(
    const sk_point3_t* location,
    sk_color_t lightColor,
    float surfaceScale, 
    float kd,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_spot_lit_diffuse(
    const sk_point3_t* location,
    const sk_point3_t* target,
    float specularExponent,
    float cutoffAngle,
    sk_color_t lightColor, 
    float surfaceScale,
    float kd,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_distant_lit_specular(
    const sk_point3_t* direction,
    sk_color_t lightColor, 
    float surfaceScale,
    float ks,
    float shininess, 
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_point_lit_specular(
    const sk_point3_t* location,
    sk_color_t lightColor, 
    float surfaceScale, 
    float ks,
    float shininess, 
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_spot_lit_specular(
    const sk_point3_t* location,
    const sk_point3_t* target, 
    float specularExponent, 
    float cutoffAngle,
    sk_color_t lightColor, 
    float surfaceScale,
    float ks,
    float shininess,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_magnifier(
    const sk_rect_t* src, 
    float inset,
    sk_imagefilter_t* input /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_matrix_convolution(
    const sk_isize_t* kernelSize,
    const float kernel[],
    float gain,
    float bias,
    const sk_ipoint_t* kernelOffset,
    sk_matrix_convolution_tilemode_t tileMode,
    bool convolveAlpha,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_merge(
    sk_imagefilter_t* filters[],
    int count,
    const sk_xfermode_mode_t modes[] /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_dilate(
    int radiusX, 
    int radiusY, 
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_erode(
    int radiusX,
    int radiusY,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_offset(
    float dx,
    float dy,
    sk_imagefilter_t* input /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_picture(
    sk_picture_t* picture);

SK_API sk_imagefilter_t* sk_imagefilter_new_picture_with_croprect(
    sk_picture_t* picture,
    const sk_rect_t* cropRect);

SK_API sk_imagefilter_t* sk_imagefilter_new_picture_for_localspace(
    sk_picture_t* picture,
    const sk_rect_t* cropRect,
    sk_filter_quality_t filterQuality);

SK_API sk_imagefilter_t* sk_imagefilter_new_rect_shader(
    sk_shader_t* shader,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

SK_API sk_imagefilter_t* sk_imagefilter_new_tile(
    const sk_rect_t* src,
    const sk_rect_t* dst,
    sk_imagefilter_t* input);

SK_API sk_imagefilter_t* sk_imagefilter_new_xfermode(
    sk_xfermode_mode_t mode, 
    sk_imagefilter_t* background,
    sk_imagefilter_t* foreground /*NULL*/,
    const sk_imagefilter_croprect_t* cropRect /*NULL*/);

///////////////////////////////////////////////////////////////////////////////////////////

SK_API void sk_colorfilter_unref(sk_colorfilter_t* filter);

SK_API sk_colorfilter_t* sk_colorfilter_new_mode(sk_color_t c, sk_xfermode_mode_t mode);

SK_API sk_colorfilter_t* sk_colorfilter_new_lighting(sk_color_t mul, sk_color_t add);

SK_API sk_colorfilter_t* sk_colorfilter_new_compose(sk_colorfilter_t* outer, sk_colorfilter_t* inner);

SK_API sk_colorfilter_t* sk_colorfilter_new_color_cube(sk_data_t* cubeData, int cubeDimension);

SK_API sk_colorfilter_t* sk_colorfilter_new_color_matrix(const float array[20]);

SK_API sk_colorfilter_t* sk_colorfilter_new_luma_color();

SK_API sk_colorfilter_t* sk_colorfilter_new_table(const uint8_t table[256]);

SK_API sk_colorfilter_t* sk_colorfilter_new_table_argb(const uint8_t tableA[256], const uint8_t tableR[256], const uint8_t tableG[256], const uint8_t tableB[256]);

///////////////////////////////////////////////////////////////////////////////////////////

SK_C_PLUS_PLUS_END_GUARD

#endif
