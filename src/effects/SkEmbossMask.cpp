/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkEmbossMask.h"

#include "include/core/SkMath.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTo.h"
#include "src/core/SkMathPriv.h"

static inline int nonzero_to_one(int x) {
#if 0
    return x != 0;
#else
    return ((unsigned)(x | -x)) >> 31;
#endif
}

static inline int neq_to_one(int x, int max) {
#if 0
    return x != max;
#else
    SkASSERT(x >= 0 && x <= max);
    return ((unsigned)(x - max)) >> 31;
#endif
}

static inline int neq_to_mask(int x, int max) {
#if 0
    return -(x != max);
#else
    SkASSERT(x >= 0 && x <= max);
    return (x - max) >> 31;
#endif
}

static inline unsigned div255(unsigned x) {
    SkASSERT(x <= (255*255));
    return x * ((1 << 24) / 255) >> 24;
}

#define kDelta  32  // small enough to show off angle differences

void SkEmbossMask::Emboss(SkMask* mask, const SkEmbossMaskFilter::Light& light) {
    SkASSERT(mask->fFormat == SkMask::k3D_Format);

    int     specular = light.fSpecular;
    int     ambient = light.fAmbient;
    SkFixed lx = SkScalarToFixed(light.fDirection[0]);
    SkFixed ly = SkScalarToFixed(light.fDirection[1]);
    SkFixed lz = SkScalarToFixed(light.fDirection[2]);
    SkFixed lz_dot_nz = lz * kDelta;
    int     lz_dot8 = lz >> 8;

    size_t      planeSize = mask->computeImageSize();
    uint8_t*    alpha = mask->fImage;
    uint8_t*    multiply = (uint8_t*)alpha + planeSize;
    uint8_t*    additive = multiply + planeSize;

    int rowBytes = mask->fRowBytes;
    int maxy = mask->fBounds.height() - 1;
    int maxx = mask->fBounds.width() - 1;

    int prev_row = 0;
    for (int y = 0; y <= maxy; y++) {
        int next_row = neq_to_mask(y, maxy) & rowBytes;

        for (int x = 0; x <= maxx; x++) {
            int nx = alpha[x + neq_to_one(x, maxx)] - alpha[x - nonzero_to_one(x)];
            int ny = alpha[x + next_row] - alpha[x - prev_row];

            SkFixed numer = lx * nx + ly * ny + lz_dot_nz;
            int     mul = ambient;
            int     add = 0;

            if (numer > 0) {  // preflight when numer/denom will be <= 0
                int denom = SkSqrt32(nx * nx + ny * ny + kDelta*kDelta);
                SkFixed dot = numer / denom;
                dot >>= 8;  // now dot is 2^8 instead of 2^16
                mul = SkMin32(mul + dot, 255);

                // now for the reflection

                //  R = 2 (Light * Normal) Normal - Light
                //  hilite = R * Eye(0, 0, 1)

                int hilite = (2 * dot - lz_dot8) * lz_dot8 >> 8;
                if (hilite > 0) {
                    // pin hilite to 255, since our fast math is also a little sloppy
                    hilite = SkClampMax(hilite, 255);

                    // specular is 4.4
                    // would really like to compute the fractional part of this
                    // and then possibly cache a 256 table for a given specular
                    // value in the light, and just pass that in to this function.
                    add = hilite;
                    for (int i = specular >> 4; i > 0; --i) {
                        add = div255(add * hilite);
                    }
                }
            }
            multiply[x] = SkToU8(mul);
            additive[x] = SkToU8(add);
        }
        alpha += rowBytes;
        multiply += rowBytes;
        additive += rowBytes;
        prev_row = rowBytes;
    }
}
