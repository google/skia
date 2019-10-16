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

sk_shader_t* sk_shader_new_empty() {
    return ToShader(SkShaders::Empty().release());
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return ToShader(SkShaders::Color(color).release());
}

sk_shader_t* sk_shader_new_blend(sk_blendmode_t cmode, sk_shader_t* dst, sk_shader_t* src, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return ToShader(SkShaders::Blend(
        (SkBlendMode)cmode, sk_ref_sp(AsShader(dst)), sk_ref_sp(AsShader(src)), localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return ToShader(AsBitmap(src)->makeShader((SkTileMode)tmx, (SkTileMode)tmy, localMatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_picture(sk_picture_t* src, sk_shader_tilemode_t tmx, sk_shader_tilemode_t tmy, const sk_matrix_t* localMatrix, const sk_rect_t* tile) {
    SkMatrix m;
    if (localMatrix) {
        m = AsMatrix(localMatrix);
    }
    return ToShader(AsPicture(src)->makeShader((SkTileMode)tmx, (SkTileMode)tmy, localMatrix ? &m : nullptr, AsRect(tile)).release());
}

sk_shader_t* sk_shader_new_color_filter(sk_shader_t* proxy, sk_colorfilter_t* filter) {
    return ToShader(AsShader(proxy)->makeWithColorFilter(sk_ref_sp(AsColorFilter(filter))).release());
}

sk_shader_t* sk_shader_new_local_matrix(sk_shader_t* proxy, const sk_matrix_t* localMatrix) {
    return ToShader(AsShader(proxy)->makeWithLocalMatrix(AsMatrix(localMatrix)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* ctileSize) {
    return ToShader(SkPerlinNoiseShader::MakeFractalNoise(
        baseFrequencyX, baseFrequencyY, numOctaves, seed, AsISize(ctileSize)).release());
}

sk_shader_t* sk_shader_new_perlin_noise_turbulence(float baseFrequencyX, float baseFrequencyY, int numOctaves, float seed, const sk_isize_t* ctileSize) {
    return ToShader(SkPerlinNoiseShader::MakeTurbulence(
        baseFrequencyX, baseFrequencyY,  numOctaves,  seed,  AsISize(ctileSize)).release());
}

void sk_shader_ref(sk_shader_t* cshader) {
    SkSafeRef(AsShader(cshader));
}

void sk_shader_unref(sk_shader_t* cshader) {
    SkSafeUnref(AsShader(cshader));
}

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t pts[2], const sk_color_t* colors, const float* colorPos, int colorCount, sk_shader_tilemode_t cmode, const sk_matrix_t* cmatrix) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    return ToShader(SkGradientShader::MakeLinear(
        AsPoint(pts), colors, colorPos, colorCount, (SkTileMode)cmode, 0, cmatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* ccenter, float radius, const sk_color_t* colors, const float* colorPos, int colorCount, sk_shader_tilemode_t cmode, const sk_matrix_t* cmatrix) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    return ToShader(SkGradientShader::MakeRadial(
        *AsPoint(ccenter), (SkScalar)radius, colors, colorPos, colorCount, (SkTileMode)cmode, 0, cmatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* ccenter, const sk_color_t* colors, const float* colorPos, int colorCount, sk_shader_tilemode_t cmode, float startAngle, float endAngle, const sk_matrix_t* cmatrix) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    return ToShader(SkGradientShader::MakeSweep(
        ccenter->x, ccenter->y, colors, colorPos, colorCount, (SkTileMode)cmode, startAngle, endAngle, 0, cmatrix ? &m : nullptr).release());
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start, float startRadius, const sk_point_t* end, float endRadius, const sk_color_t* colors, const float* colorPos, int colorCount, sk_shader_tilemode_t cmode, const sk_matrix_t* cmatrix) {
    SkMatrix m;
    if (cmatrix) {
        m = AsMatrix(cmatrix);
    }
    return ToShader(SkGradientShader::MakeTwoPointConical(
        *AsPoint(start), startRadius, *AsPoint(end), endRadius, colors, colorPos, colorCount, (SkTileMode)cmode, 0, cmatrix ? &m : nullptr).release());
}
