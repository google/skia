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
#include "SkHalf.h"
#include "SkSRGB.h"
#include "SkTemplates.h"

namespace SK_OPTS_NS {

// Strange that we need a wrapper on SkNx_cast to use as a function ptr.
static Sk4i Sk4f_trunc(const Sk4f& x) {
    return SkNx_cast<int>(x);
}

static Sk4f linear_to_2dot2(const Sk4f& x) {
    // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
    auto x2  = x.rsqrt(),                            // x^(-1/2)
         x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
         x64 = x32.rsqrt();                          // x^(+1/64)

    // 29 = 32 - 2 - 1
    return 255.0f * x2.invert() * x32 * x64.invert();
}

enum DstGamma {
    // 8888
    kSRGB_DstGamma,
    k2Dot2_DstGamma,
    kTable_DstGamma,

    // F16
    kLinear_DstGamma,
};

template <DstGamma kDstGamma>
static void color_xform_RGB1(void* dst, const uint32_t* src, int len,
                             const float* const srcTables[3], const float matrix[16],
                             const uint8_t* const dstTables[3]) {
    Sk4f rXgXbX = Sk4f::Load(matrix +  0),
         rYgYbY = Sk4f::Load(matrix +  4),
         rZgZbZ = Sk4f::Load(matrix +  8),
         rTgTbT = Sk4f::Load(matrix + 12);

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
                            &rYgYbY, &rZgZbZ, &rTgTbT] {
            dstReds   = rXgXbX[0]*reds + rYgYbY[0]*greens + rZgZbZ[0]*blues + rTgTbT[0];
            dstGreens = rXgXbX[1]*reds + rYgYbY[1]*greens + rZgZbZ[1]*blues + rTgTbT[1];
            dstBlues  = rXgXbX[2]*reds + rYgYbY[2]*greens + rZgZbZ[2]*blues + rTgTbT[2];
        };

        auto store_4 = [&dstReds, &dstGreens, &dstBlues, &dst, &dstTables] {
            if (kSRGB_DstGamma == kDstGamma || k2Dot2_DstGamma == kDstGamma) {
                Sk4f (*linear_to_curve)(const Sk4f&) = (kSRGB_DstGamma == kDstGamma) ?
                        sk_linear_to_srgb_needs_trunc : linear_to_2dot2;
                Sk4i (*float_to_int)(const Sk4f&) = (kSRGB_DstGamma == kDstGamma) ?
                        Sk4f_trunc : Sk4f_round;

                dstReds   = linear_to_curve(dstReds);
                dstGreens = linear_to_curve(dstGreens);
                dstBlues  = linear_to_curve(dstBlues);

                dstReds   = sk_clamp_0_255(dstReds);
                dstGreens = sk_clamp_0_255(dstGreens);
                dstBlues  = sk_clamp_0_255(dstBlues);

                auto rgba = (float_to_int(dstReds)   << SK_R32_SHIFT)
                          | (float_to_int(dstGreens) << SK_G32_SHIFT)
                          | (float_to_int(dstBlues)  << SK_B32_SHIFT)
                          | (Sk4i{0xFF}              << SK_A32_SHIFT);
                rgba.store((uint32_t*) dst);

                dst = SkTAddOffset<void>(dst, 4 * sizeof(uint32_t));
            } else if (kTable_DstGamma == kDstGamma) {
                Sk4f scaledReds   = Sk4f::Min(Sk4f::Max(1023.0f * dstReds,   0.0f), 1023.0f);
                Sk4f scaledGreens = Sk4f::Min(Sk4f::Max(1023.0f * dstGreens, 0.0f), 1023.0f);
                Sk4f scaledBlues  = Sk4f::Min(Sk4f::Max(1023.0f * dstBlues,  0.0f), 1023.0f);

                Sk4i indicesReds   = Sk4f_round(scaledReds);
                Sk4i indicesGreens = Sk4f_round(scaledGreens);
                Sk4i indicesBlues  = Sk4f_round(scaledBlues);

                uint32_t* dst32 = (uint32_t*) dst;
                dst32[0] = dstTables[0][indicesReds  [0]] << SK_R32_SHIFT
                         | dstTables[1][indicesGreens[0]] << SK_G32_SHIFT
                         | dstTables[2][indicesBlues [0]] << SK_B32_SHIFT
                         | 0xFF                           << SK_A32_SHIFT;
                dst32[1] = dstTables[0][indicesReds  [1]] << SK_R32_SHIFT
                         | dstTables[1][indicesGreens[1]] << SK_G32_SHIFT
                         | dstTables[2][indicesBlues [1]] << SK_B32_SHIFT
                         | 0xFF                           << SK_A32_SHIFT;
                dst32[2] = dstTables[0][indicesReds  [2]] << SK_R32_SHIFT
                         | dstTables[1][indicesGreens[2]] << SK_G32_SHIFT
                         | dstTables[2][indicesBlues [2]] << SK_B32_SHIFT
                         | 0xFF                           << SK_A32_SHIFT;
                dst32[3] = dstTables[0][indicesReds  [3]] << SK_R32_SHIFT
                         | dstTables[1][indicesGreens[3]] << SK_G32_SHIFT
                         | dstTables[2][indicesBlues [3]] << SK_B32_SHIFT
                         | 0xFF                           << SK_A32_SHIFT;

                dst = SkTAddOffset<void>(dst, 4 * sizeof(uint32_t));
            } else {
                Sk4h_store4(dst, SkFloatToHalf_finite(dstReds),
                                 SkFloatToHalf_finite(dstGreens),
                                 SkFloatToHalf_finite(dstBlues),
                                 SK_Half1);
                dst = SkTAddOffset<void>(dst, 4 * sizeof(uint64_t));
            }
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

        auto dstPixel = rXgXbX*r + rYgYbY*g + rZgZbZ*b + rTgTbT;

        if (kSRGB_DstGamma == kDstGamma || k2Dot2_DstGamma == kDstGamma) {
            Sk4f (*linear_to_curve)(const Sk4f&) = (kSRGB_DstGamma == kDstGamma) ?
                    sk_linear_to_srgb_needs_trunc : linear_to_2dot2;
            Sk4i (*float_to_int)(const Sk4f&) = (kSRGB_DstGamma == kDstGamma) ?
                    Sk4f_trunc : Sk4f_round;

            dstPixel = sk_clamp_0_255(linear_to_curve(dstPixel));

            uint32_t rgba;
            SkNx_cast<uint8_t>(float_to_int(dstPixel)).store(&rgba);
            rgba |= 0xFF000000;
            *((uint32_t*) dst) = SkSwizzle_RGBA_to_PMColor(rgba);
            dst = SkTAddOffset<void>(dst, sizeof(uint32_t));
        } else if (kTable_DstGamma == kDstGamma) {
            Sk4f scaledPixel = Sk4f::Min(Sk4f::Max(1023.0f * dstPixel, 0.0f), 1023.0f);

            Sk4i indices = Sk4f_round(scaledPixel);

            *((uint32_t*) dst) = dstTables[0][indices[0]] << SK_R32_SHIFT
                               | dstTables[1][indices[1]] << SK_G32_SHIFT
                               | dstTables[2][indices[2]] << SK_B32_SHIFT
                               | 0xFF                     << SK_A32_SHIFT;

            dst = SkTAddOffset<void>(dst, sizeof(uint32_t));
        } else {
            uint64_t rgba;
            SkFloatToHalf_finite(dstPixel).store(&rgba);
            rgba |= static_cast<uint64_t>(SK_Half1) << 48;
            *((uint64_t*) dst) = rgba;
            dst = SkTAddOffset<void>(dst, sizeof(uint64_t));
        }

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

static void color_xform_RGB1_to_linear(uint64_t* dst, const uint32_t* src, int len,
                                       const float* const srcTables[3], const float matrix[16]) {
    color_xform_RGB1<kLinear_DstGamma>(dst, src, len, srcTables, matrix, nullptr);
}

}  // namespace SK_OPTS_NS

#endif // SkColorXform_opts_DEFINED
