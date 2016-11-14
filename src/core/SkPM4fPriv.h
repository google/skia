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
#include "SkFixedAlloc.h"
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


// N.B. scratch_matrix_3x4 must live at least as long as p.
static inline bool append_gamut_transform(SkRasterPipeline* p, float scratch_matrix_3x4[12],
                                          SkColorSpace* src, SkColorSpace* dst) {
    if (src == dst) { return true; }
    if (!dst)       { return true; }   // Legacy modes intentionally ignore color gamut.
    if (!src)       { return true; }   // A null src color space means linear gamma, dst gamut.

    auto toXYZ = as_CSB(src)->  toXYZD50(),
       fromXYZ = as_CSB(dst)->fromXYZD50();
    if (!toXYZ || !fromXYZ) { return false; }  // Unsupported color space type.

    if (as_CSB(src)->toXYZD50Hash() == as_CSB(dst)->toXYZD50Hash()) { return true; }

    SkMatrix44 m44(*fromXYZ, *toXYZ);

    // Convert from 4x4 to (column-major) 3x4.
    auto ptr = scratch_matrix_3x4;
    *ptr++ = m44.get(0,0); *ptr++ = m44.get(1,0); *ptr++ = m44.get(2,0);
    *ptr++ = m44.get(0,1); *ptr++ = m44.get(1,1); *ptr++ = m44.get(2,1);
    *ptr++ = m44.get(0,2); *ptr++ = m44.get(1,2); *ptr++ = m44.get(2,2);
    *ptr++ = m44.get(0,3); *ptr++ = m44.get(1,3); *ptr++ = m44.get(2,3);

    p->append(SkRasterPipeline::matrix_3x4, scratch_matrix_3x4);
    // TODO: detect whether we can skip the clamps?
    p->append(SkRasterPipeline::clamp_0);
    p->append(SkRasterPipeline::clamp_a);
    return true;
}

static inline bool append_gamut_transform(SkRasterPipeline* p, SkFallbackAlloc* scratch,
                                          SkColorSpace* src, SkColorSpace* dst) {
    struct matrix_3x4 { float arr[12]; };
    return append_gamut_transform(p, scratch->make<matrix_3x4>()->arr, src, dst);
}

static inline SkPM4f SkPM4f_from_SkColor(SkColor color, SkColorSpace* dst) {
    SkColor4f color4f;
    if (dst) {
        // sRGB gamma, sRGB gamut.
        color4f = SkColor4f::FromColor(color);
        void* color4f_ptr = &color4f;

        float scratch_matrix_3x4[12];

        SkRasterPipeline p;
        p.append(SkRasterPipeline::constant_color, color4f_ptr);
        append_gamut_transform(&p, scratch_matrix_3x4,
                               SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named).get(), dst);
        p.append(SkRasterPipeline::store_f32, &color4f_ptr);

        p.compile()(0,1);
    } else {
        // Linear gamma, dst gamut.
        swizzle_rb(SkNx_cast<float>(Sk4b::Load(&color)) * (1/255.0f)).store(&color4f);
    }
    return color4f.premul();
}

#endif
