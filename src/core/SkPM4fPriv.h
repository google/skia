/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkNx.h"

static inline float get_alpha(const Sk4f& f4) {
    return f4.kth<SkPM4f::A>();
}

static inline Sk4f set_alpha(const Sk4f& f4, float alpha) {
    static_assert(3 == SkPM4f::A, "");
    return Sk4f(f4.kth<0>(), f4.kth<1>(), f4.kth<2>(), alpha);
}

static inline uint32_t to_4b(const Sk4f& f4) {
    uint32_t b4;
    SkNx_cast<uint8_t>(f4).store((uint8_t*)&b4);
    return b4;
}

static inline Sk4f to_4f(uint32_t b4) {
    return SkNx_cast<float>(Sk4b::Load((const uint8_t*)&b4));
}

static inline Sk4f s2l(const Sk4f& s4) {
    return set_alpha(s4 * s4, get_alpha(s4));
}

static inline Sk4f l2s(const Sk4f& l4) {
    return set_alpha(l4.rsqrt1() * l4, get_alpha(l4));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline Sk4f Sk4f_fromL32(uint32_t src) {
    return to_4f(src) * Sk4f(1.0f/255);
}

static inline Sk4f Sk4f_fromS32(uint32_t src) {
    return s2l(to_4f(src) * Sk4f(1.0f/255));
}

static inline uint32_t Sk4f_toL32(const Sk4f& x4) {
    return to_4b(x4 * Sk4f(255) + Sk4f(0.5f));
}

static inline uint32_t Sk4f_toS32(const Sk4f& x4) {
    return to_4b(l2s(x4) * Sk4f(255) + Sk4f(0.5f));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static Sk4f unit_to_l255_round(const SkPM4f& pm4) {
    return Sk4f::Load(pm4.fVec) * Sk4f(255) + Sk4f(0.5f);
}

static Sk4f unit_to_s255_round(const SkPM4f& pm4) {
    return l2s(Sk4f::Load(pm4.fVec)) * Sk4f(255) + Sk4f(0.5f);
}

static inline void SkPM4f_l32_src_mode(SkPMColor dst[], const SkPM4f src[], int count) {
    for (int i = 0; i < (count >> 2); ++i) {
        SkASSERT(src[0].isUnit());
        SkASSERT(src[1].isUnit());
        SkASSERT(src[2].isUnit());
        SkASSERT(src[3].isUnit());
        Sk4f_ToBytes((uint8_t*)dst,
                     unit_to_l255_round(src[0]), unit_to_l255_round(src[1]),
                     unit_to_l255_round(src[2]), unit_to_l255_round(src[3]));
        src += 4;
        dst += 4;
    }
    count &= 3;
    for (int i = 0; i < count; ++i) {
        SkASSERT(src[i].isUnit());
        SkNx_cast<uint8_t>(unit_to_l255_round(src[i])).store((uint8_t*)&dst[i]);
    }
}

static inline void SkPM4f_l32_srcover_mode(SkPMColor dst[], const SkPM4f src[], int count) {
    for (int i = 0; i < count; ++i) {
        SkASSERT(src[i].isUnit());
        Sk4f s4 = Sk4f::Load(src[i].fVec);
        Sk4f d4 = Sk4f_fromL32(dst[i]);
        dst[i] = Sk4f_toL32(s4 + d4 * Sk4f(1 - get_alpha(s4)));
    }
}

static inline void SkPM4f_s32_src_mode(SkPMColor dst[], const SkPM4f src[], int count) {
    for (int i = 0; i < (count >> 2); ++i) {
        SkASSERT(src[0].isUnit());
        SkASSERT(src[1].isUnit());
        SkASSERT(src[2].isUnit());
        SkASSERT(src[3].isUnit());
        Sk4f_ToBytes((uint8_t*)dst,
                     unit_to_s255_round(src[0]), unit_to_s255_round(src[1]),
                     unit_to_s255_round(src[2]), unit_to_s255_round(src[3]));
        src += 4;
        dst += 4;
    }
    count &= 3;
    for (int i = 0; i < count; ++i) {
        SkASSERT(src[i].isUnit());
        SkNx_cast<uint8_t>(unit_to_s255_round(src[i])).store((uint8_t*)&dst[i]);
    }
}

static inline void SkPM4f_s32_srcover_mode(SkPMColor dst[], const SkPM4f src[], int count) {
    for (int i = 0; i < count; ++i) {
        SkASSERT(src[i].isUnit());
        Sk4f s4 = Sk4f::Load(src[i].fVec);
        Sk4f d4 = Sk4f_fromS32(dst[i]);
        dst[i] = Sk4f_toS32(s4 + d4 * Sk4f(1 - get_alpha(s4)));
    }
}

