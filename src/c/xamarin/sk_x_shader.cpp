/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShader.h"
#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkPerlinNoiseShader.h"

#include "xamarin/sk_x_shader.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

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
        SkXfermode::kSrcOver_Mode);
    return ToShader(s.release());
}

sk_shader_t* sk_shader_new_compose_with_mode(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB,
    sk_xfermode_mode_t cmode) {

    sk_sp<SkShader> s = SkShader::MakeComposeShader(
        sk_ref_sp(AsShader(shaderA)),
        sk_ref_sp(AsShader(shaderB)),
        (SkXfermode::Mode)cmode);
    return ToShader(s.release());
}
