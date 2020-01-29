/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_shader_DEFINED
#define sk_shader_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

// SkShader

SK_C_API void sk_shader_ref(sk_shader_t* shader);
SK_C_API void sk_shader_unref(sk_shader_t* shader);
SK_C_API sk_shader_t* sk_shader_with_local_matrix(const sk_shader_t* shader, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_with_color_filter(const sk_shader_t* shader, const sk_colorfilter_t* filter);

// SkShaders

SK_C_API sk_shader_t* sk_shader_new_empty(void);
SK_C_API sk_shader_t* sk_shader_new_color(sk_color_t color);
SK_C_API sk_shader_t* sk_shader_new_color4f(const sk_color4f_t* color, const sk_colorspace_t* colorspace);
SK_C_API sk_shader_t* sk_shader_new_compose(const sk_shader_t* shaderA, const sk_shader_t* shaderB, sk_blendmode_t mode);
SK_C_API sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_picture(sk_picture_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix, const sk_rect_t* tile);

// SkGradientShader

SK_C_API sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t points[2], const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_linear_gradient_color4f(const sk_point_t points[2], const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* center, float radius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_radial_gradient_color4f(const sk_point_t* center, float radius, const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* center, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, float startAngle, float endAngle, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_sweep_gradient_color4f(const sk_point_t* center, const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, float startAngle, float endAngle, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix);
SK_C_API sk_shader_t* sk_shader_new_two_point_conical_gradient_color4f(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix);

// SkPerlinNoiseShader

SK_C_API sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize);
SK_C_API sk_shader_t* sk_shader_new_perlin_noise_turbulence(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize);
SK_C_API sk_shader_t* sk_shader_new_perlin_noise_improved_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float z);

SK_C_PLUS_PLUS_END_GUARD

#endif
