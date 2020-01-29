/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShader.h"
#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkPerlinNoiseShader.h"

#include "sk_shader.h"

#include "sk_types_priv.h"

// SkShader

void sk_shader_ref(sk_shader_t* shader){
    SkSafeRef(AsShader(shader));
}

void sk_shader_unref(sk_shader_t* shader){
    SkSafeUnref(AsShader(shader));
}

sk_shader_t* sk_shader_with_local_matrix(const sk_shader_t* shader, const sk_matrix_t* localMatrix) {
    return ToShader(AsShader(shader)->makeWithLocalMatrix(AsMatrix(localMatrix)).release());
}

sk_shader_t* sk_shader_with_color_filter(const sk_shader_t* shader, const sk_colorfilter_t* filter) {
    return ToShader(AsShader(shader)->makeWithColorFilter(sk_ref_sp(AsColorFilter(filter))).release());
}

// SkShaders

sk_shader_t* sk_shader_new_empty(void) {
    return ToShader(SkShader::MakeEmptyShader().release());
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return ToShader(SkShader::MakeColorShader(color).release());
}

sk_shader_t* sk_shader_new_color4f(const sk_color4f_t* color, const sk_colorspace_t* colorspace) {
    return ToShader(SkShader::MakeColorShader(*AsColor4f(color), sk_ref_sp(AsColorSpace(colorspace))).release());
}

sk_shader_t* sk_shader_new_compose(const sk_shader_t* shaderA, const sk_shader_t* shaderB, sk_blendmode_t mode) {
    return ToShader(SkShader::MakeComposeShader(sk_ref_sp(AsShader(shaderA)), sk_ref_sp(AsShader(shaderB)), (SkBlendMode)mode).release());
}

sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkShader::MakeBitmapShader(*AsBitmap(src), (SkShader::TileMode)tmx, (SkShader::TileMode)tmy, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_picture(sk_picture_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix, const sk_rect_t* tile) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkShader::MakePictureShader(sk_ref_sp(AsPicture(src)), (SkShader::TileMode)tmx, (SkShader::TileMode)tmy, localMatrix ? &m : nullptr, AsRect(tile)).release());
}

// SkGradientShader

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t points[2], const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeLinear(AsPoint(points), colors, colorPos, colorCount, (SkShader::TileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_linear_gradient_color4f(const sk_point_t points[2], const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeLinear(AsPoint(points), AsColor4f(colors), sk_ref_sp(AsColorSpace(colorspace)), colorPos, colorCount, (SkShader::TileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* center, float radius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeRadial(*AsPoint(center), (SkScalar)radius, colors, colorPos, colorCount, (SkShader::TileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_radial_gradient_color4f(const sk_point_t* center, float radius, const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeRadial(*AsPoint(center), (SkScalar)radius, AsColor4f(colors), sk_ref_sp(AsColorSpace(colorspace)), colorPos, colorCount, (SkShader::TileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* center, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, float startAngle, float endAngle, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeSweep(center->x, center->y, colors, colorPos, colorCount, (SkShader::TileMode)tileMode, startAngle, endAngle, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_sweep_gradient_color4f(const sk_point_t* center, const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, float startAngle, float endAngle, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeSweep(center->x, center->y, AsColor4f(colors), sk_ref_sp(AsColorSpace(colorspace)), colorPos, colorCount, (SkShader::TileMode)tileMode, startAngle, endAngle, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color_t colors[], const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeTwoPointConical(*AsPoint(start), startRadius, *AsPoint(end), endRadius, colors, colorPos, colorCount, (SkShader::TileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_two_point_conical_gradient_color4f(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color4f_t* colors, const sk_colorspace_t* colorspace, const float colorPos[], int colorCount, sk_shader_tilemode_t tileMode, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix)
        m = AsMatrix(localMatrix);
    return ToShader(SkGradientShader::MakeTwoPointConical(*AsPoint(start), startRadius, *AsPoint(end), endRadius, AsColor4f(colors), sk_ref_sp(AsColorSpace(colorspace)), colorPos, colorCount, (SkShader::TileMode)tileMode, 0, localMatrix ? &m : nullptr).release());
}

// SkPerlinNoiseShader

sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize) {
    return ToShader(SkPerlinNoiseShader::MakeFractalNoise(baseFrequencyX, baseFrequencyY, numOctaves, seed, AsISize(tileSize)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_turbulence(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* tileSize) {
    return ToShader(SkPerlinNoiseShader::MakeTurbulence(baseFrequencyX, baseFrequencyY,  numOctaves,  seed,  AsISize(tileSize)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_improved_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float z) {
    return ToShader(SkPerlinNoiseShader::MakeImprovedNoise(baseFrequencyX, baseFrequencyY, numOctaves, z).release());
}
