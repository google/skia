// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SkColorCubeFilter_opts_DEFINED
#define SkColorCubeFilter_opts_DEFINED

#include "SkColor.h"
#include "SkPMFloat.h"
#include "SkUnPreMultiply.h"

namespace SK_OPTS_NS {

void color_cube_filter_span(const SkPMColor src[],
                            int count,
                            SkPMColor dst[],
                            const int* colorToIndex[2],
                            const SkScalar* colorToFactors[2],
                            int dim,
                            const SkColor* colorCube) {
    uint8_t* ptr_dst = reinterpret_cast<uint8_t*>(dst);
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

        SkPMFloat color(0,0,0,0);

        for (int x = 0; x < 2; ++x) {
            const int ix = colorToIndex[x][r];

            const SkColor lutColor00 = colorCube[ix + i00];
            const SkColor lutColor01 = colorCube[ix + i01];
            const SkColor lutColor10 = colorCube[ix + i10];
            const SkColor lutColor11 = colorCube[ix + i11];

            Sk4f  sum = SkPMFloat::FromOpaqueColor(lutColor00) * g0b0;
            sum = sum + SkPMFloat::FromOpaqueColor(lutColor01) * g0b1;
            sum = sum + SkPMFloat::FromOpaqueColor(lutColor10) * g1b0;
            sum = sum + SkPMFloat::FromOpaqueColor(lutColor11) * g1b1;

            color = color + sum * Sk4f((float)colorToFactors[x][r]);
        }

        if (a != 255) {
            color = color * Sk4f(a * 1.0f/255);
        }

        dst[i] = color.round();

        ptr_dst[SK_A32_SHIFT / 8] = a;
        ptr_dst += 4;
    }
}

}  // namespace SK_OPTS NS

#endif  // SkColorCubeFilter_opts_DEFINED
