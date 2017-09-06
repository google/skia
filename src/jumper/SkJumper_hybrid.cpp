/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper_hybrid.h"

STAGE_GG(seed_shader, Ctx::None,
         int x, int y, XY* xy) {
    float x_offsets[] = {
         0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
         8.5, 9.5,10.5,11.5,12.5,13.5,14.5,15.5,
        16.5,17.5,18.5,19.5,20.5,21.5,22.5,23.5,
        24.5,25.5,26.5,27.5,28.6,29.5,30.5,31.5,
    };
    using F = decltype(xy->x);
    xy->x = F(x) + load<F>(x_offsets);
    xy->y = F(y) + 0.5;
}

STAGE_PP_H(uniform_color, const SkJumper_UniformColorCtx* ctx,
           int x, int y, Pixel* src, Pixel* dst) {
    src->r = ctx->r;
    src->g = ctx->g;
    src->b = ctx->b;
    src->a = ctx->a;
}

STAGE_PP_L(uniform_color, const SkJumper_UniformColorCtx* ctx,
           int x, int y, Pixel* src, Pixel* dst) {
#if defined(HYBRID_IS_NEON)
    auto rgba = vld4_dup_u8((const uint8_t*)&ctx->rgba);
    src->r = rgba.val[0];
    src->g = rgba.val[1];
    src->b = rgba.val[2];
    src->a = rgba.val[3];
#else
    src->rgba = ctx->rgba;
#endif
}
