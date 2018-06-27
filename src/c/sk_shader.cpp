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

sk_shader_t* sk_shader_new_empty() {
    return ToShader(SkShader::MakeEmptyShader().release());
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return ToShader(SkShader::MakeColorShader(color).release());
}

sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t* src,
                                  sk_shader_tilemode_t tmx,
                                  sk_shader_tilemode_t tmy,
                                  const sk_matrix_t* localMatrix) {
    SkMatrix matrix;
    if (localMatrix) {
        from_c(localMatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    sk_sp<SkShader> s = SkShader::MakeBitmapShader(
        *AsBitmap(src),
        (SkShader::TileMode)tmx,
        (SkShader::TileMode)tmy,
        &matrix);
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_picture(sk_picture_t* src,
                                  sk_shader_tilemode_t tmx,
                                  sk_shader_tilemode_t tmy,
                                  const sk_matrix_t* localMatrix,
                                  const sk_rect_t* tile) {
    SkMatrix matrix;
    if (localMatrix) {
        from_c(localMatrix, &matrix);
    }
    else {
        matrix.setIdentity();
    }
    sk_sp<SkShader> s = SkShader::MakePictureShader(
        sk_ref_sp(AsPicture(src)),
        (SkShader::TileMode)tmx,
        (SkShader::TileMode)tmy,
        &matrix,
        AsRect(tile));
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_color_filter(sk_shader_t* proxy,
                                        sk_colorfilter_t* filter) {
    sk_sp<SkShader> s = AsShader(proxy)->makeWithColorFilter(sk_ref_sp(AsColorFilter(filter)));
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_local_matrix(sk_shader_t* proxy,
                                        const sk_matrix_t* localMatrix) {
    SkMatrix matrix;
    if (localMatrix) {
        from_c(localMatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    sk_sp<SkShader> s = AsShader(proxy)->makeWithLocalMatrix(matrix);
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(
    float baseFrequencyX,
    float baseFrequencyY,
    int numOctaves,
    float seed,
    const sk_isize_t* ctileSize) {

    const SkISize* tileSize = AsISize(ctileSize);
    sk_sp<SkShader> s = SkPerlinNoiseShader::MakeFractalNoise(
        baseFrequencyX,
        baseFrequencyY,
        numOctaves,
        seed,
        tileSize);
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_perlin_noise_turbulence(
    float baseFrequencyX,
    float baseFrequencyY,
    int numOctaves,
    float seed,
    const sk_isize_t* ctileSize) {

    const SkISize* tileSize = AsISize(ctileSize);
    sk_sp<SkShader> s = SkPerlinNoiseShader::MakeTurbulence(
        baseFrequencyX,
        baseFrequencyY, 
        numOctaves, 
        seed, 
        tileSize);
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_compose(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB) {

    sk_sp<SkShader> s = SkShader::MakeComposeShader(
        sk_ref_sp(AsShader(shaderA)),
        sk_ref_sp(AsShader(shaderB)),
        SkBlendMode::kSrcOver);
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_compose_with_mode(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB,
    sk_blendmode_t cmode) {

    sk_sp<SkShader> s = SkShader::MakeComposeShader(
        sk_ref_sp(AsShader(shaderA)),
        sk_ref_sp(AsShader(shaderB)),
        (SkBlendMode)cmode);
    return ToShader(s.release());
}

void sk_shader_ref(sk_shader_t* cshader) {
    SkSafeRef(AsShader(cshader));
}

void sk_shader_unref(sk_shader_t* cshader) {
    SkSafeUnref(AsShader(cshader));
}

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t pts[2],
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    return (sk_shader_t*)SkGradientShader::MakeLinear(reinterpret_cast<const SkPoint*>(pts),
                                                      reinterpret_cast<const SkColor*>(colors),
                                                      colorPos, colorCount,
                                                      (SkShader::TileMode)cmode, 0, &matrix).release();
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* ccenter,
                                           float radius,
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkPoint center = AsPoint(*ccenter);
    return (sk_shader_t*)SkGradientShader::MakeRadial(center, (SkScalar)radius,
                                                      reinterpret_cast<const SkColor*>(colors),
                                                      reinterpret_cast<const SkScalar*>(colorPos),
                                                      colorCount, (SkShader::TileMode)cmode, 0, &matrix).release();
}

sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* ccenter,
                                          const sk_color_t colors[],
                                          const float colorPos[],
                                          int colorCount,
                                          const sk_matrix_t* cmatrix) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    return (sk_shader_t*)SkGradientShader::MakeSweep((SkScalar)(ccenter->x),
                                                     (SkScalar)(ccenter->y),
                                                     reinterpret_cast<const SkColor*>(colors),
                                                     reinterpret_cast<const SkScalar*>(colorPos),
                                                     colorCount, 0, &matrix).release();
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start,
                                                      float startRadius,
                                                      const sk_point_t* end,
                                                      float endRadius,
                                                      const sk_color_t colors[],
                                                      const float colorPos[],
                                                      int colorCount,
                                                      sk_shader_tilemode_t cmode,
                                                      const sk_matrix_t* cmatrix) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkPoint skstart = AsPoint(*start);
    SkPoint skend = AsPoint(*end);
    return (sk_shader_t*)SkGradientShader::MakeTwoPointConical(skstart, (SkScalar)startRadius,
                                                        skend, (SkScalar)endRadius,
                                                        reinterpret_cast<const SkColor*>(colors),
                                                        reinterpret_cast<const SkScalar*>(colorPos),
                                                        colorCount, (SkShader::TileMode)cmode, 0, &matrix).release();
}
