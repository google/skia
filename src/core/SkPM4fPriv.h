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
#include "SkColorSpaceXformSteps.h"
#include "SkArenaAlloc.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"
#include "../jumper/SkJumper.h"

// This file is mostly helper routines for doing color space management.
// It probably wants a new name, and they likely don't need to be inline.
//
// There are two generations of routines in here, the old ones that assumed linear blending,
// and the new ones assuming as-encoded blending.  We're trying to move to the new as-encoded
// ones and will hopefully eventually remove all the linear routines.
//
// We'll start with the new as-encoded routines first,
// and shove all the old broken routines towards the bottom.

static inline Sk4f Sk4f_fromL32(uint32_t px) {
    return SkNx_cast<float>(Sk4b::Load(&px)) * (1/255.0f);
}

static inline uint32_t Sk4f_toL32(const Sk4f& px) {
    uint32_t l32;
    SkNx_cast<uint8_t>(Sk4f_round(px * 255.0f)).store(&l32);
    return l32;
}

static inline SkPM4f premul_in_dst_colorspace(SkColor4f color4f,
                                              SkColorSpace* srcCS, SkColorSpace* dstCS) {
    // We treat untagged sources as sRGB.
    if (!srcCS) { srcCS = SkColorSpace::MakeSRGB().get(); }

    // If dstCS is null, no color space transformation is needed (and apply() will just premul).
    if (!dstCS) { dstCS = srcCS; }

    // TODO: In the very common case of srcCS being sRGB,
    // can we precompute an sRGB -> dstCS SkColorSpaceXformSteps for each device and use it here?
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType, dstCS)
        .apply(color4f.vec());

    return {{color4f.fR, color4f.fG, color4f.fB, color4f.fA}};
}

static inline SkPM4f premul_in_dst_colorspace(SkColor c, SkColorSpace* dstCS) {
    SkColor4f color4f;
    swizzle_rb(Sk4f_fromL32(c)).store(color4f.vec());

    // SkColors are always sRGB.
    return premul_in_dst_colorspace(color4f, SkColorSpace::MakeSRGB().get(), dstCS);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Functions below this line are probably totally broken as far as color space management goes.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static inline Sk4f Sk4f_fromS32(uint32_t px) {
    return { sk_linear_from_srgb[(px >>  0) & 0xff],
             sk_linear_from_srgb[(px >>  8) & 0xff],
             sk_linear_from_srgb[(px >> 16) & 0xff],
                    (1/255.0f) * (px >> 24)          };
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


#endif
