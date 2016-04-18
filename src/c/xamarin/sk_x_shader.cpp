/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShader.h"
#include "SkComposeShader.h"
#include "..\..\include\effects\SkPerlinNoiseShader.h"

#include "xamarin\sk_x_shader.h"

#include "..\sk_types_priv.h"
#include "sk_x_types_priv.h"

sk_shader_t* sk_shader_new_empty() {
    return (sk_shader_t*) SkShader::CreateEmptyShader();
}

sk_shader_t* sk_shader_new_color(sk_color_t color) {
    return (sk_shader_t*) SkShader::CreateColorShader(color);
}

sk_shader_t* sk_shader_new_bitmap(const sk_bitmap_t* src,
                                  sk_shader_tilemode_t tmx,
                                  sk_shader_tilemode_t tmy,
                                  const sk_matrix_t* localMatrix) {
    SkShader::TileMode modex;
    if (!find_sk(tmx, &modex)) {
        return NULL;
    }
    SkShader::TileMode modey;
    if (!find_sk(tmy, &modey)) {
        return NULL;
    }
    SkMatrix matrix;
    if (localMatrix) {
        from_c(localMatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = SkShader::CreateBitmapShader(*AsBitmap(src), modex, modey, &matrix);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_picture(const sk_picture_t* src,
                                  sk_shader_tilemode_t tmx,
                                  sk_shader_tilemode_t tmy,
                                  const sk_matrix_t* localMatrix,
                                  const sk_rect_t* tile) {
    SkShader::TileMode modex;
    if (!find_sk(tmx, &modex)) {
        return NULL;
    }
    SkShader::TileMode modey;
    if (!find_sk(tmy, &modey)) {
        return NULL;
    }
    SkMatrix matrix;
    if (localMatrix) {
        from_c(localMatrix, &matrix);
    }
    else {
        matrix.setIdentity();
    }
    SkShader* s = SkShader::CreatePictureShader(AsPicture(src), modex, modey, &matrix, AsRect(tile));
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_color_filter(sk_shader_t* proxy,
                                        sk_colorfilter_t* filter) {
    SkShader* s = AsShader(proxy)->newWithColorFilter(AsColorFilter(filter));
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_local_matrix(sk_shader_t* proxy,
                                        const sk_matrix_t* localMatrix) {
    SkMatrix matrix;
    if (localMatrix) {
        from_c(localMatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkShader* s = AsShader(proxy)->newWithLocalMatrix(matrix);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_perlin_noise_fractal_noise(
    float baseFrequencyX,
    float baseFrequencyY,
    int numOctaves,
    float seed,
    const sk_isize_t* ctileSize) {

    const SkISize* tileSize = AsISize(ctileSize);
    SkShader* s = SkPerlinNoiseShader::CreateFractalNoise(
        baseFrequencyX,
        baseFrequencyY,
        numOctaves,
        seed,
        tileSize);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_perlin_noise_turbulence(
    float baseFrequencyX,
    float baseFrequencyY,
    int numOctaves,
    float seed,
    const sk_isize_t* ctileSize) {

    const SkISize* tileSize = AsISize(ctileSize);
    SkShader* s = SkPerlinNoiseShader::CreateTurbulence(
        baseFrequencyX,
        baseFrequencyY, 
        numOctaves, 
        seed, 
        tileSize);
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_compose(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB) {

    SkShader* s = new SkComposeShader(AsShader(shaderA), AsShader(shaderB));
    return (sk_shader_t*)s;
}

sk_shader_t* sk_shader_new_compose_with_mode(
    sk_shader_t* shaderA,
    sk_shader_t* shaderB,
    sk_xfermode_mode_t cmode) {

    SkXfermode::Mode mode;
    if (!find_sk(cmode, &mode)) {
        return NULL;
    }
    SkShader* s = new SkComposeShader(AsShader(shaderA), AsShader(shaderB), SkXfermode::Create(mode));
    return (sk_shader_t*)s;
}
