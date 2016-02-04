/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_shader_DEFINED
#define sk_shader_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_shader_ref(sk_shader_t*);
SK_API void sk_shader_unref(sk_shader_t*);

/**
    Call this to create a new "empty" shader, that will not draw anything.
*/
SK_API sk_shader_t* sk_shader_new_empty();

/**
    Call this to create a new shader that just draws the specified color. This should always
    draw the same as a paint with this color (and no shader).
*/
SK_API sk_shader_t* sk_shader_new_color(sk_color_t color);

/** 
    Call this to create a new shader that will draw with the specified bitmap.
 
    If the bitmap cannot be used (e.g. has no pixels, or its dimensions
    exceed implementation limits (currently at 64K - 1)) then SkEmptyShader
    may be returned.
 
    @param src  The bitmap to use inside the shader
    @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
    @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
*/
SK_API sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t& src,
                                         sk_shader_tilemode_t tmx,
                                         sk_shader_tilemode_t tmy,
                                         const sk_matrix_t* localMatrix);

SK_API sk_shader_t* sk_shader_new_picture(const sk_picture_t* src,
                                         sk_shader_tilemode_t tmx,
                                         sk_shader_tilemode_t tmy,
                                         const sk_matrix_t* localMatrix,
                                         const sk_rect_t* tile);

/**
    Return a shader that will apply the specified localMatrix to the proxy shader.
    The specified matrix will be applied before any matrix associated with the proxy.

    Note: ownership of the proxy is not transferred (though a ref is taken).
*/
SK_API sk_shader_t* sk_shader_new_local_matrix(sk_shader_t* proxy,
                                               const sk_matrix_t* localMatrix);

SK_API sk_shader_t* sk_shader_new_color_filter(sk_shader_t* proxy,
                                               sk_colorfilter_t* filter);

/**
    Returns a shader that generates a linear gradient between the two
    specified points.

    @param points The start and end points for the gradient.
    @param colors The array[count] of colors, to be distributed between
                  the two points
    @param colorPos May be NULL. array[count] of SkScalars, or NULL, of
                    the relative position of each corresponding color
                    in the colors array. If this is NULL, the the
                    colors are distributed evenly between the start
                    and end point.  If this is not null, the values
                    must begin with 0, end with 1.0, and intermediate
                    values must be strictly increasing.
    @param colorCount Must be >=2. The number of colors (and pos if not
                      NULL) entries.
    @param mode The tiling mode
*/
SK_API sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t points[2],
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t tileMode,
                                           const sk_matrix_t* localMatrix);


/**
    Returns a shader that generates a radial gradient given the center
    and radius.

    @param center The center of the circle for this gradient
    @param radius Must be positive. The radius of the circle for this
                  gradient
    @param colors The array[count] of colors, to be distributed
                  between the center and edge of the circle
    @param colorPos May be NULL. The array[count] of the relative
                    position of each corresponding color in the colors
                    array. If this is NULL, the the colors are
                    distributed evenly between the center and edge of
                    the circle.  If this is not null, the values must
                    begin with 0, end with 1.0, and intermediate
                    values must be strictly increasing.
    @param count Must be >= 2. The number of colors (and pos if not
                 NULL) entries
    @param tileMode The tiling mode
    @param localMatrix May be NULL
*/
SK_API sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* center,
                                           float radius,
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t tileMode,
                                           const sk_matrix_t* localMatrix);

/**
    Returns a shader that generates a sweep gradient given a center.

    @param center The coordinates of the center of the sweep
    @param colors The array[count] of colors, to be distributed around
                  the center.
    @param colorPos May be NULL. The array[count] of the relative
                    position of each corresponding color in the colors
                    array. If this is NULL, the the colors are
                    distributed evenly between the center and edge of
                    the circle.  If this is not null, the values must
                    begin with 0, end with 1.0, and intermediate
                    values must be strictly increasing.
    @param colorCount Must be >= 2. The number of colors (and pos if
                      not NULL) entries
    @param localMatrix May be NULL
*/
SK_API sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* center,
                                          const sk_color_t colors[],
                                          const float colorPos[],
                                          int colorCount,
                                          const sk_matrix_t* localMatrix);

/**
    Returns a shader that generates a conical gradient given two circles, or
    returns NULL if the inputs are invalid. The gradient interprets the
    two circles according to the following HTML spec.
    http://dev.w3.org/html5/2dcontext/#dom-context-2d-createradialgradient

    Returns a shader that generates a sweep gradient given a center.

    @param start, startRadius Defines the first circle.
    @param end, endRadius Defines the first circle.
    @param colors The array[count] of colors, to be distributed between
                  the two circles.
    @param colorPos May be NULL. The array[count] of the relative
                    position of each corresponding color in the colors
                    array. If this is NULL, the the colors are
                    distributed evenly between the two circles.  If
                    this is not null, the values must begin with 0,
                    end with 1.0, and intermediate values must be
                    strictly increasing.
    @param colorCount Must be >= 2. The number of colors (and pos if
                      not NULL) entries
    @param tileMode The tiling mode
    @param localMatrix May be NULL

*/
SK_API sk_shader_t* sk_shader_new_two_point_conical_gradient(
        const sk_point_t* start,
        float startRadius,
        const sk_point_t* end,
        float endRadius,
        const sk_color_t colors[],
        const float colorPos[],
        int colorCount,
        sk_shader_tilemode_t tileMode,
        const sk_matrix_t* localMatrix);

SK_API sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(
    float baseFrequencyX,
    float baseFrequencyY,
    int numOctaves,
    float seed,
    const sk_isize_t* tileSize);

SK_API sk_shader_t* sk_shader_new_perlin_noise_turbulence(
    float baseFrequencyX,
    float baseFrequencyY,
    int numOctaves,
    float seed,
    const sk_isize_t* tileSize);

SK_API sk_shader_t* sk_shader_new_compose(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB);

SK_API sk_shader_t* sk_shader_new_compose_with_mode(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB,
    sk_xfermode_mode_t mode);

SK_C_PLUS_PLUS_END_GUARD

#endif
