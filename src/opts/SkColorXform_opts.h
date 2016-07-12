/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorXform_opts_DEFINED
#define SkColorXform_opts_DEFINED

#include "SkNx.h"
#include "SkColorPriv.h"
#include "SkSRGB.h"

namespace SK_OPTS_NS {

static Sk4f linear_to_2dot2(const Sk4f& x) {
    // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
    auto x2  = x.rsqrt(),                            // x^(-1/2)
         x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
         x64 = x32.rsqrt();                          // x^(+1/64)

    // 29 = 32 - 2 - 1
    return 255.0f * x2.invert() * x32 * x64.invert();
}

static Sk4f clamp_0_to_255(const Sk4f& x) {
    // The order of the arguments is important here.  We want to make sure that NaN
    // clamps to zero.  Note that max(NaN, 0) = 0, while max(0, NaN) = NaN.
    return Sk4f::Min(Sk4f::Max(x, 0.0f), 255.0f);
}

enum DstGamma {
    kSRGB_DstGamma,
    k2Dot2_DstGamma,
    kTable_DstGamma,
};

template <DstGamma kDstGamma>
static void color_xform_RGB1(uint32_t* dst, const uint32_t* src, int len,
                             const float* const srcTables[3], const float matrix[16],
                             const uint8_t* const dstTables[3]) {
    Sk4f rXgXbX = Sk4f::Load(matrix + 0),
         rYgYbY = Sk4f::Load(matrix + 4),
         rZgZbZ = Sk4f::Load(matrix + 8);

    if (len >= 4) {
        Sk4f reds, greens, blues;
        auto load_next_4 = [&reds, &greens, &blues, &src, &len, &srcTables] {
            reds   = Sk4f{srcTables[0][(src[0] >>  0) & 0xFF],
                          srcTables[0][(src[1] >>  0) & 0xFF],
                          srcTables[0][(src[2] >>  0) & 0xFF],
                          srcTables[0][(src[3] >>  0) & 0xFF]};
            greens = Sk4f{srcTables[1][(src[0] >>  8) & 0xFF],
                          srcTables[1][(src[1] >>  8) & 0xFF],
                          srcTables[1][(src[2] >>  8) & 0xFF],
                          srcTables[1][(src[3] >>  8) & 0xFF]};
            blues  = Sk4f{srcTables[2][(src[0] >> 16) & 0xFF],
                          srcTables[2][(src[1] >> 16) & 0xFF],
                          srcTables[2][(src[2] >> 16) & 0xFF],
                          srcTables[2][(src[3] >> 16) & 0xFF]};
            src += 4;
            len -= 4;
        };

        Sk4f dstReds, dstGreens, dstBlues;
        auto transform_4 = [&reds, &greens, &blues, &dstReds, &dstGreens, &dstBlues, &rXgXbX,
                            &rYgYbY, &rZgZbZ] {
            dstReds   = rXgXbX[0]*reds + rYgYbY[0]*greens + rZgZbZ[0]*blues;
            dstGreens = rXgXbX[1]*reds + rYgYbY[1]*greens + rZgZbZ[1]*blues;
            dstBlues  = rXgXbX[2]*reds + rYgYbY[2]*greens + rZgZbZ[2]*blues;
        };

        auto store_4 = [&dstReds, &dstGreens, &dstBlues, &dst, &dstTables] {
            if (kSRGB_DstGamma == kDstGamma || k2Dot2_DstGamma == kDstGamma) {
                Sk4f (*linear_to_curve)(const Sk4f&) =
                        (kSRGB_DstGamma == kDstGamma) ? sk_linear_to_srgb : linear_to_2dot2;

                dstReds   = linear_to_curve(dstReds);
                dstGreens = linear_to_curve(dstGreens);
                dstBlues  = linear_to_curve(dstBlues);

                dstReds   = clamp_0_to_255(dstReds);
                dstGreens = clamp_0_to_255(dstGreens);
                dstBlues  = clamp_0_to_255(dstBlues);

                auto rgba = (Sk4f_round(dstReds)        )
                          | (Sk4f_round(dstGreens) <<  8)
                          | (Sk4f_round(dstBlues)  << 16)
                          | (Sk4i{      0xFF       << 24});
                rgba.store(dst);
            } else {
                Sk4f scaledReds   = Sk4f::Min(Sk4f::Max(1023.0f * dstReds,   0.0f), 1023.0f);
                Sk4f scaledGreens = Sk4f::Min(Sk4f::Max(1023.0f * dstGreens, 0.0f), 1023.0f);
                Sk4f scaledBlues  = Sk4f::Min(Sk4f::Max(1023.0f * dstBlues,  0.0f), 1023.0f);

                Sk4i indicesReds   = Sk4f_round(scaledReds);
                Sk4i indicesGreens = Sk4f_round(scaledGreens);
                Sk4i indicesBlues  = Sk4f_round(scaledBlues);

                dst[0] = dstTables[0][indicesReds  [0]]
                       | dstTables[1][indicesGreens[0]] <<  8
                       | dstTables[2][indicesBlues [0]] << 16
                       | 0xFF                           << 24;
                dst[1] = dstTables[0][indicesReds  [1]]
                       | dstTables[1][indicesGreens[1]] <<  8
                       | dstTables[2][indicesBlues [1]] << 16
                       | 0xFF                           << 24;
                dst[2] = dstTables[0][indicesReds  [2]]
                       | dstTables[1][indicesGreens[2]] <<  8
                       | dstTables[2][indicesBlues [2]] << 16
                       | 0xFF                           << 24;
                dst[3] = dstTables[0][indicesReds  [3]]
                       | dstTables[1][indicesGreens[3]] <<  8
                       | dstTables[2][indicesBlues [3]] << 16
                       | 0xFF                           << 24;
            }

            dst += 4;
        };

        load_next_4();

        while (len >= 4) {
            transform_4();
            load_next_4();
            store_4();
        }

        transform_4();
        store_4();
    }

    while (len > 0) {
        // Splat r,g,b across a register each.
        auto r = Sk4f{srcTables[0][(*src >>  0) & 0xFF]},
             g = Sk4f{srcTables[1][(*src >>  8) & 0xFF]},
             b = Sk4f{srcTables[2][(*src >> 16) & 0xFF]};

        // Apply transformation matrix to dst gamut.
        auto dstPixel = rXgXbX*r + rYgYbY*g + rZgZbZ*b;

        if (kSRGB_DstGamma == kDstGamma || k2Dot2_DstGamma == kDstGamma) {
            Sk4f (*linear_to_curve)(const Sk4f&) =
                    (kSRGB_DstGamma == kDstGamma) ? sk_linear_to_srgb : linear_to_2dot2;

            dstPixel = linear_to_curve(dstPixel);

            dstPixel = clamp_0_to_255(dstPixel);

            uint32_t rgba;
            SkNx_cast<uint8_t>(Sk4f_round(dstPixel)).store(&rgba);
            rgba |= 0xFF000000;
            *dst = rgba;
        } else {
            Sk4f scaledPixel = Sk4f::Min(Sk4f::Max(1023.0f * dstPixel, 0.0f), 1023.0f);

            Sk4i indices = Sk4f_round(scaledPixel);

            *dst = dstTables[0][indices[0]]
                 | dstTables[1][indices[1]] <<  8
                 | dstTables[2][indices[2]] << 16
                 | 0xFF                     << 24;
        }

        dst += 1;
        src += 1;
        len -= 1;
    }
}

static void color_xform_RGB1_to_2dot2(uint32_t* dst, const uint32_t* src, int len,
                                      const float* const srcTables[3], const float matrix[16]) {
    color_xform_RGB1<k2Dot2_DstGamma>(dst, src, len, srcTables, matrix, nullptr);
}

static void color_xform_RGB1_to_srgb(uint32_t* dst, const uint32_t* src, int len,
                                     const float* const srcTables[3], const float matrix[16]) {
    color_xform_RGB1<kSRGB_DstGamma>(dst, src, len, srcTables, matrix, nullptr);
}

static void color_xform_RGB1_to_table(uint32_t* dst, const uint32_t* src, int len,
                                      const float* const srcTables[3], const float matrix[16],
                                      const uint8_t* const dstTables[3]) {
    color_xform_RGB1<kTable_DstGamma>(dst, src, len, srcTables, matrix, dstTables);
}

}  // namespace SK_OPTS_NS

#endif // SkColorXform_opts_DEFINED
