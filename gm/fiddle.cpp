/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"

#pragma clang attribute push(__attribute__((target("avx2,f16c,fma"))), apply_to=function)
#define __SSE__ 1
#define __SSE2__ 1
#define __SSE3__ 1
#define __SSSE3__ 1
#define __SSE4_1__ 1
#define __SSE4_2__ 1
#define __AVX__ 1
#define __F16C__ 1
#define __AVX2__ 1
#define __FMA__ 1

#include <immintrin.h>

using F32 = float __attribute__((ext_vector_type(8)));

DEF_SIMPLE_GM(fiddle, canvas, 256, 256) {
    auto cube = [](F32 x) {
        return _mm256_mul_ps(x,x) * x;
    };

    float w = canvas->getBaseLayerSize().width();
    F32 x = {0+w,1+w,2+w,3+w, 4+w,5+w,6+w,7+w};

    canvas->clear(SkColorSetA(cube(x)[7], 0xff));  // 0xff159457, a pretty green
}

#pragma clang attribute pop
