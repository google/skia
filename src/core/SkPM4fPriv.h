/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4fPriv_DEFINED
#define SkPM4fPriv_DEFINED

#include "SkColorData.h"
#include "SkColorSpace.h"
#include "SkArenaAlloc.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"
#include "../jumper/SkJumper.h"

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

static inline void append_gamut_transform(SkRasterPipeline* p,
                                          SkArenaAlloc* alloc,
                                          SkColorSpace* src,
                                          SkColorSpace* dst,
                                          SkAlphaType srcAT) {
    if (src == dst || !dst || !src) {
        return;
    }

    const SkMatrix44 *fromSrc = src->  toXYZD50(),
                       *toDst = dst->fromXYZD50();
    if (!fromSrc || !toDst) {
        SkDEBUGFAIL("We can't handle non-XYZ color spaces in append_gamut_transform().");
        return;
    }

    // Slightly more sophisticated version of if (src == dst)
    if (src->toXYZD50Hash() == dst->toXYZD50Hash()) {
        return;
    }

    // Convert from 4x4 to (column-major) 3x4.
    SkMatrix44 m44(*toDst, *fromSrc);
    float* ptr = alloc->makeArrayDefault<float>(12);
    *ptr++ = m44.get(0,0); *ptr++ = m44.get(1,0); *ptr++ = m44.get(2,0);
    *ptr++ = m44.get(0,1); *ptr++ = m44.get(1,1); *ptr++ = m44.get(2,1);
    *ptr++ = m44.get(0,2); *ptr++ = m44.get(1,2); *ptr++ = m44.get(2,2);
    *ptr++ = m44.get(0,3); *ptr++ = m44.get(1,3); *ptr++ = m44.get(2,3);
    p->append(SkRasterPipeline::matrix_3x4, ptr-12);
}

static inline SkColor4f to_colorspace(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
    SkColor4f color4f = c;
    if (src && dst && !SkColorSpace::Equals(src, dst)) {
        SkJumper_MemoryCtx color4f_ptr = { &color4f, 0 };

        SkSTArenaAlloc<256> alloc;
        SkRasterPipeline p(&alloc);
        p.append_constant_color(&alloc, color4f);
        append_gamut_transform(&p, &alloc, src, dst, kUnpremul_SkAlphaType);
        p.append(SkRasterPipeline::store_f32, &color4f_ptr);

        p.run(0,0,1,1);
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
