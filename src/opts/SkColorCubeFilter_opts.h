// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SkColorCubeFilter_opts_DEFINED
#define SkColorCubeFilter_opts_DEFINED

#include "SkColor.h"
#include "SkNx.h"
#include "SkUnPreMultiply.h"

namespace SK_OPTS_NS {

void color_cube_filter_span(const SkPMColor src[],
                            int count,
                            SkPMColor dst[],
                            const int* colorToIndex[2],
                            const SkScalar* colorToFactors[2],
                            int dim,
                            const SkColor* colorCube) {
    uint8_t r, g, b, a;

    for (int i = 0; i < count; ++i) {
        const SkPMColor input = src[i];
        a = input >> SK_A32_SHIFT;

        if (a != 255) {
            const SkColor source = SkUnPreMultiply::PMColorToColor(input);
            r = SkColorGetR(source);
            g = SkColorGetG(source);
            b = SkColorGetB(source);
        } else {
            r = SkGetPackedR32(input);
            g = SkGetPackedG32(input);
            b = SkGetPackedB32(input);
        }

        const SkScalar g0 = colorToFactors[0][g],
                       g1 = colorToFactors[1][g],
                       b0 = colorToFactors[0][b],
                       b1 = colorToFactors[1][b];

        const Sk4f g0b0(g0*b0),
                   g0b1(g0*b1),
                   g1b0(g1*b0),
                   g1b1(g1*b1);

        const int i00 = (colorToIndex[0][g] + colorToIndex[0][b] * dim) * dim;
        const int i01 = (colorToIndex[0][g] + colorToIndex[1][b] * dim) * dim;
        const int i10 = (colorToIndex[1][g] + colorToIndex[0][b] * dim) * dim;
        const int i11 = (colorToIndex[1][g] + colorToIndex[1][b] * dim) * dim;

        Sk4f color(0.5f);  // Starting from 0.5f gets us rounding for free.
        for (int x = 0; x < 2; ++x) {
            const int ix = colorToIndex[x][r];

            const SkColor lutColor00 = colorCube[ix + i00];
            const SkColor lutColor01 = colorCube[ix + i01];
            const SkColor lutColor10 = colorCube[ix + i10];
            const SkColor lutColor11 = colorCube[ix + i11];

            Sk4f  sum = SkNx_cast<float>(Sk4b::Load(&lutColor00)) * g0b0;
            sum = sum + SkNx_cast<float>(Sk4b::Load(&lutColor01)) * g0b1;
            sum = sum + SkNx_cast<float>(Sk4b::Load(&lutColor10)) * g1b0;
            sum = sum + SkNx_cast<float>(Sk4b::Load(&lutColor11)) * g1b1;
            color = color + sum * Sk4f((float)colorToFactors[x][r]);
        }
        if (a != 255) {
            color = color * Sk4f(a * (1.0f/255));
        }

        // color is BGRA (SkColor order), dst is SkPMColor order, so may need to swap R+B.
    #if defined(SK_PMCOLOR_IS_RGBA)
        color = SkNx_shuffle<2,1,0,3>(color);
    #endif
        uint8_t* dstBytes = (uint8_t*)(dst+i);
        SkNx_cast<uint8_t>(color).store(dstBytes);
        dstBytes[SK_A32_SHIFT/8] = a;
    }
}

}  // namespace SK_OPTS NS

#endif  // SkColorCubeFilter_opts_DEFINED
