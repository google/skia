/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_shader_DEFINED
#define sk_x_shader_DEFINED

#include "sk_shader.h"

#include "sk_types.h"
#include "xamarin/sk_x_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

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
SK_API sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t* src,
                                         sk_shader_tilemode_t tmx,
                                         sk_shader_tilemode_t tmy,
                                         const sk_matrix_t* localMatrix);
SK_API sk_shader_t* sk_shader_new_picture(sk_picture_t* src,
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
