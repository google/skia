/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPicture.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"

#include "include/c/sk_shader.h"

#include "src/c/sk_types_priv.h"

// SkShader

void sk_shader_ref(sk_shader_t* shader) {
    SkSafeRef(AsShader(shader));
}

void sk_shader_unref(sk_shader_t* shader) {
    SkSafeUnref(AsShader(shader));
}


sk_shader_t* sk_shader_with_local_matrix(sk_shader_t* shader, const sk_matrix_t* localMatrix) {
    return ToShader(AsShader(shader)->makeWithLocalMatrix(AsMatrix(localMatrix)).release());
}

sk_shader_t* sk_shader_with_color_filter(sk_shader_t* shader, sk_colorfilter_t* filter) {
    return ToShader(AsShader(shader)->makeWithColorFilter(sk_ref_sp(AsColorFilter(filter))).release());
}


sk_shader_t* sk_shader_new_empty(void) {
    return ToShader(SkShaders::Empty().release());
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return ToShader(SkShaders::Color(color).release());
}

sk_shader_t* sk_shader_new_blend(sk_blendmode_t mode, sk_shader_t* dst, sk_shader_t* src, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkShaders::Blend(
        (SkBlendMode)mode, sk_ref_sp(AsShader(dst)), sk_ref_sp(AsShader(src)), localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_lerp(float t, sk_shader_t* dst, sk_shader_t* src, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkShaders::Lerp(
        t, sk_ref_sp(AsShader(dst)), sk_ref_sp(AsShader(src)), localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_lerp_red(sk_shader_t* red, sk_shader_t* dst, sk_shader_t* src, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkShaders::Lerp(
        sk_ref_sp(AsShader(red)), sk_ref_sp(AsShader(dst)), sk_ref_sp(AsShader(src)), localMatrix ? &m : nullptr).release());
}


// SkShaders

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t points[2], const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeLinear(
        AsPoint(points), colors, colorPos, colorCount, (SkTileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* center, float radius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeRadial(
        *AsPoint(center), (SkScalar)radius, colors, colorPos, colorCount, (SkTileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeTwoPointConical(
        *AsPoint(start), startRadius, *AsPoint(end), endRadius, colors, colorPos, colorCount, (SkTileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_sweep_gradient(float cx, float cy, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, float startAngle, float endAngle, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeSweep(
        cx, cy, colors, colorPos, colorCount, (SkTileMode)tileMode, startAngle, endAngle, 0, localMatrix ? &m : nullptr).release());
}


sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(float baseFrequencyX, float baseFrequencyY,int numOctaves, float seed, const sk_isize_t* tileSize) {
    return ToShader(SkPerlinNoiseShader::MakeFractalNoise(
        baseFrequencyX, baseFrequencyY, numOctaves, seed, AsISize(tileSize)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_turbulence(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize) {
    return ToShader(SkPerlinNoiseShader::MakeTurbulence(
        baseFrequencyX, baseFrequencyY, numOctaves, seed, AsISize(tileSize)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_improved_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float z) {
    return ToShader(SkPerlinNoiseShader::MakeImprovedNoise(
        baseFrequencyX, baseFrequencyY, numOctaves, z).release());
}
