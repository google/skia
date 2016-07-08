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

extern const float sk_linear_from_srgb[256];
extern const float sk_linear_from_2dot2[256];

namespace SK_OPTS_NS {

static Sk4f linear_to_2dot2(const Sk4f& x) {
    // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
    auto x2  = x.rsqrt(),                            // x^(-1/2)
         x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
         x64 = x32.rsqrt();                          // x^(+1/64)

    // 29 = 32 - 2 - 1
    return 255.0f * x2.invert() * x32 * x64.invert();
}

static Sk4f linear_to_srgb(const Sk4f& x) {
    // Approximation of the sRGB gamma curve (within 1 when scaled to 8-bit pixels).
    // For 0.00000f <= x <  0.00349f,    12.92 * x
    // For 0.00349f <= x <= 1.00000f,    0.679*(x.^0.5) + 0.423*x.^(0.25) - 0.101
    // Note that 0.00349 was selected because it is a point where both functions produce the
    // same pixel value when rounded.
    auto rsqrt = x.rsqrt(),
         sqrt  = rsqrt.invert(),
         ftrt  = rsqrt.rsqrt();

    auto hi = (-0.101115084998961f * 255.0f) +
              (+0.678513029959381f * 255.0f) * sqrt +
              (+0.422602055039580f * 255.0f) * ftrt;

    auto lo = (12.92f * 255.0f) * x;

    auto mask = (x < 0.00349f);
    return mask.thenElse(lo, hi);
}

static Sk4f clamp_0_to_255(const Sk4f& x) {
    // The order of the arguments is important here.  We want to make sure that NaN
    // clamps to zero.  Note that max(NaN, 0) = 0, while max(0, NaN) = NaN.
    return Sk4f::Min(Sk4f::Max(x, 0.0f), 255.0f);
}

template <const float (&linear_from_curve)[256], Sk4f (*linear_to_curve)(const Sk4f&)>
static void color_xform_RGB1(uint32_t* dst, const uint32_t* src, int len,
                             const float matrix[16]) {
    Sk4f rXgXbX = Sk4f::Load(matrix + 0),
         rYgYbY = Sk4f::Load(matrix + 4),
         rZgZbZ = Sk4f::Load(matrix + 8);

    if (len >= 4) {
        Sk4f reds, greens, blues;
        auto load_next_4 = [&reds, &greens, &blues, &src, &len] {
            reds   = Sk4f{linear_from_curve[(src[0] >>  0) & 0xFF],
                          linear_from_curve[(src[1] >>  0) & 0xFF],
                          linear_from_curve[(src[2] >>  0) & 0xFF],
                          linear_from_curve[(src[3] >>  0) & 0xFF]};
            greens = Sk4f{linear_from_curve[(src[0] >>  8) & 0xFF],
                          linear_from_curve[(src[1] >>  8) & 0xFF],
                          linear_from_curve[(src[2] >>  8) & 0xFF],
                          linear_from_curve[(src[3] >>  8) & 0xFF]};
            blues  = Sk4f{linear_from_curve[(src[0] >> 16) & 0xFF],
                          linear_from_curve[(src[1] >> 16) & 0xFF],
                          linear_from_curve[(src[2] >> 16) & 0xFF],
                          linear_from_curve[(src[3] >> 16) & 0xFF]};
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

        auto store_4 = [&dstReds, &dstGreens, &dstBlues, &dst] {
            dstReds   = linear_to_curve(dstReds);
            dstGreens = linear_to_curve(dstGreens);
            dstBlues  = linear_to_curve(dstBlues);

            dstReds   = clamp_0_to_255(dstReds);
            dstGreens = clamp_0_to_255(dstGreens);
            dstBlues  = clamp_0_to_255(dstBlues);

            auto rgba = (Sk4i{(int)0xFF000000}          )
                      | (SkNx_cast<int>(dstReds)        )
                      | (SkNx_cast<int>(dstGreens) <<  8)
                      | (SkNx_cast<int>(dstBlues)  << 16);
            rgba.store(dst);
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
        auto r = Sk4f{linear_from_curve[(*src >>  0) & 0xFF]},
             g = Sk4f{linear_from_curve[(*src >>  8) & 0xFF]},
             b = Sk4f{linear_from_curve[(*src >> 16) & 0xFF]};

        // Apply transformation matrix to dst gamut.
        auto dstPixel = rXgXbX*r + rYgYbY*g + rZgZbZ*b;

        // Convert to dst gamma.
        dstPixel = linear_to_curve(dstPixel);

        // Clamp floats to byte range.
        dstPixel = clamp_0_to_255(dstPixel);

        // Convert to bytes and store to memory.
        uint32_t rgba;
        SkNx_cast<uint8_t>(dstPixel).store(&rgba);
        rgba |= 0xFF000000;
        *dst = rgba;

        dst += 1;
        src += 1;
        len -= 1;
    }
}

static void color_xform_RGB1_srgb_to_2dot2(uint32_t* dst, const uint32_t* src, int len,
                                           const float matrix[16]) {
    color_xform_RGB1<sk_linear_from_srgb, linear_to_2dot2>(dst, src, len, matrix);
}

static void color_xform_RGB1_2dot2_to_2dot2(uint32_t* dst, const uint32_t* src, int len,
                                           const float matrix[16]) {
    color_xform_RGB1<sk_linear_from_2dot2, linear_to_2dot2>(dst, src, len, matrix);
}

static void color_xform_RGB1_srgb_to_srgb(uint32_t* dst, const uint32_t* src, int len,
                                           const float matrix[16]) {
    color_xform_RGB1<sk_linear_from_srgb, linear_to_srgb>(dst, src, len, matrix);
}

static void color_xform_RGB1_2dot2_to_srgb(uint32_t* dst, const uint32_t* src, int len,
                                           const float matrix[16]) {
    color_xform_RGB1<sk_linear_from_2dot2, linear_to_srgb>(dst, src, len, matrix);
}

}  // namespace SK_OPTS_NS

#endif // SkColorXform_opts_DEFINED
