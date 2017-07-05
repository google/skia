/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4fPriv_DEFINED
#define SkPM4fPriv_DEFINED

#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkArenaAlloc.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"

static inline Sk4f set_alpha(const Sk4f& px, float alpha) {
    return { px[0], px[1], px[2], alpha };
}

static inline float get_alpha(const Sk4f& px) {
    return px[3];
}


static inline Sk4f Sk4f_fromL32(uint32_t px) {
    return SkNx_cast<float>(Sk4b::Load(&px)) * (1/255.0f);
}

static inline Sk4f Sk4f_fromS32(uint32_t px) {
    return { sk_linear_from_srgb[(px >>  0) & 0xff],
             sk_linear_from_srgb[(px >>  8) & 0xff],
             sk_linear_from_srgb[(px >> 16) & 0xff],
                    (1/255.0f) * (px >> 24)          };
}

static inline uint32_t Sk4f_toL32(const Sk4f& px) {
    uint32_t l32;
    SkNx_cast<uint8_t>(Sk4f_round(px * 255.0f)).store(&l32);
    return l32;
}

static inline uint32_t Sk4f_toS32(const Sk4f& px) {
    Sk4i  rgb = sk_linear_to_srgb(px),
         srgb = { rgb[0], rgb[1], rgb[2], (int)(255.0f * px[3] + 0.5f) };

    uint32_t s32;
    SkNx_cast<uint8_t>(srgb).store(&s32);
    return s32;
}


// SkColor handling:
//   SkColor has an ordering of (b, g, r, a) if cast to an Sk4f, so the code swizzles r and b to
// produce the needed (r, g, b, a) ordering.
static inline Sk4f Sk4f_from_SkColor(SkColor color) {
    return swizzle_rb(Sk4f_fromS32(color));
}

static inline void assert_unit(float x) {
    SkASSERT(0 <= x && x <= 1);
}

static inline float exact_srgb_to_linear(float srgb) {
    assert_unit(srgb);
    float linear;
    if (srgb <= 0.04045) {
        linear = srgb / 12.92f;
    } else {
        linear = powf((srgb + 0.055f) / 1.055f, 2.4f);
    }
    assert_unit(linear);
    return linear;
}

static inline void analyze_3x4_matrix(const float matrix[12],
                                      bool* can_underflow, bool* can_overflow) {
    // | 0 3 6  9 |   |r|   |x|
    // | 1 4 7 10 | x |g| = |y|
    // | 2 5 8 11 |   |b|   |z|
    //                |1|
    // We'll find min/max bounds on each of x,y,z assuming r,g,b are all in [0,1].
    // If any can be <0, we'll set can_underflow; if any can be >1, can_overflow.
    bool underflow = false,
          overflow = false;
    for (int i = 0; i < 3; i++) {
        SkScalar min = matrix[i+9],
                 max = matrix[i+9];
        (matrix[i+0] < 0 ? min : max) += matrix[i+0];
        (matrix[i+3] < 0 ? min : max) += matrix[i+3];
        (matrix[i+6] < 0 ? min : max) += matrix[i+6];
        underflow = underflow || min < 0;
        overflow  =  overflow || max > 1;
    }
    *can_underflow = underflow;
    *can_overflow  =  overflow;
}


// N.B. scratch_matrix_3x4 must live at least as long as p.
static inline void append_gamut_transform(SkRasterPipeline* p, float scratch_matrix_3x4[12],
                                          SkColorSpace* src, SkColorSpace* dst,
                                          SkAlphaType alphaType) {
    if (src == dst) { return; }   // That was easy.
    if (!dst)       { return; }   // Legacy modes intentionally ignore color gamut.
    if (!src)       { return; }   // A null src color space means linear gamma, dst gamut.

    auto toXYZ = as_CSB(src)->  toXYZD50(),
       fromXYZ = as_CSB(dst)->fromXYZD50();
    if (!toXYZ || !fromXYZ) {
        SkASSERT(false);  // We really don't want to get here with a weird colorspace.
        return;
    }

    // Slightly more sophisticated version of if (src == dst)
    if (as_CSB(src)->toXYZD50Hash() == as_CSB(dst)->toXYZD50Hash()) {
        return;
    }

    SkMatrix44 m44(*fromXYZ, *toXYZ);

    // Convert from 4x4 to (column-major) 3x4.
    auto ptr = scratch_matrix_3x4;
    *ptr++ = m44.get(0,0); *ptr++ = m44.get(1,0); *ptr++ = m44.get(2,0);
    *ptr++ = m44.get(0,1); *ptr++ = m44.get(1,1); *ptr++ = m44.get(2,1);
    *ptr++ = m44.get(0,2); *ptr++ = m44.get(1,2); *ptr++ = m44.get(2,2);
    *ptr++ = m44.get(0,3); *ptr++ = m44.get(1,3); *ptr++ = m44.get(2,3);

    bool needs_clamp_0, needs_clamp_1;
    analyze_3x4_matrix(scratch_matrix_3x4, &needs_clamp_0, &needs_clamp_1);

    p->append(SkRasterPipeline::matrix_3x4, scratch_matrix_3x4);
    if (needs_clamp_0) { p->append(SkRasterPipeline::clamp_0); }
    if (needs_clamp_1) {
        (kPremul_SkAlphaType == alphaType) ? p->append(SkRasterPipeline::clamp_a)
                                           : p->append(SkRasterPipeline::clamp_1);
    }
}

static inline void append_gamut_transform(SkRasterPipeline* p, SkArenaAlloc* scratch,
                                          SkColorSpace* src, SkColorSpace* dst,
                                          SkAlphaType alphaType) {
    append_gamut_transform(p, scratch->makeArrayDefault<float>(12), src, dst, alphaType);
}

static inline SkColor4f to_colorspace(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
    SkColor4f color4f = c;
    if (src && dst) {
        void* color4f_ptr = &color4f;

        float scratch_matrix_3x4[12];

        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::uniform_color, color4f_ptr);
        append_gamut_transform(&p, scratch_matrix_3x4, src, dst, kUnpremul_SkAlphaType);
        p.append(SkRasterPipeline::store_f32, &color4f_ptr);

        p.run(0,0,1);
    }
    return color4f;
}

static inline SkColor4f SkColor4f_from_SkColor(SkColor color, SkColorSpace* dst) {
    SkColor4f color4f;
    if (dst) {
        // sRGB gamma, sRGB gamut.
        color4f = to_colorspace(SkColor4f::FromColor(color),
                                SkColorSpace::MakeSRGB().get(), dst);
    } else {
        // Linear gamma, dst gamut.
        swizzle_rb(SkNx_cast<float>(Sk4b::Load(&color)) * (1/255.0f)).store(&color4f);
    }
    return color4f;
}

static inline SkPM4f SkPM4f_from_SkColor(SkColor color, SkColorSpace* dst) {
    return SkColor4f_from_SkColor(color, dst).premul();
}

#endif
