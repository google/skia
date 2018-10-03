/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4f_DEFINED
#define SkPM4f_DEFINED

#include "SkColorData.h"
#include "SkNx.h"

static inline Sk4f swizzle_rb(const Sk4f& x) {
    return SkNx_shuffle<2, 1, 0, 3>(x);
}

static inline Sk4f swizzle_rb_if_bgra(const Sk4f& x) {
#ifdef SK_PMCOLOR_IS_BGRA
    return swizzle_rb(x);
#else
    return x;
#endif
}

static inline Sk4f Sk4f_fromL32(uint32_t px) {
    return SkNx_cast<float>(Sk4b::Load(&px)) * (1/255.0f);
}

static inline uint32_t Sk4f_toL32(const Sk4f& px) {
    Sk4f v = px;

#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    // SkNx_cast<uint8_t, int32_t>() pins, and we don't anticipate giant floats
#elif !defined(SKNX_NO_SIMD) && defined(SK_ARM_HAS_NEON)
    // SkNx_cast<uint8_t, int32_t>() pins, and so does Sk4f_round().
#else
    // No guarantee of a pin.
    v = Sk4f::Max(0, Sk4f::Min(v, 1));
#endif

    uint32_t l32;
    SkNx_cast<uint8_t>(Sk4f_round(v * 255.0f)).store(&l32);
    return l32;
}

using SkPMColor4f = SkRGBA4f<kPremul_SkAlphaType>;

constexpr SkPMColor4f SK_PMColor4fTRANSPARENT = { 0, 0, 0, 0 };
constexpr SkPMColor4f SK_PMColor4fWHITE       = { 1, 1, 1, 1 };

#endif
