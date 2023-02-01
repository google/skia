/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkColorData.h"
#include "src/base/SkVx.h"

#include <cstdint>

static inline skvx::float4 swizzle_rb(const skvx::float4& x) {
    return skvx::shuffle<2, 1, 0, 3>(x);
}

static inline skvx::float4 swizzle_rb_if_bgra(const skvx::float4& x) {
#if defined(SK_PMCOLOR_IS_BGRA)
    return swizzle_rb(x);
#else
    return x;
#endif
}

static inline skvx::float4 Sk4f_fromL32(uint32_t px) {
    return skvx::cast<float>(skvx::byte4::Load(&px)) * (1 / 255.0f);
}

static inline uint32_t Sk4f_toL32(const skvx::float4& px) {
    uint32_t l32;
    // For the expected positive color values, the +0.5 before the pin and cast effectively rounds
    // to the nearest int without having to call round() or lrint().
    skvx::cast<uint8_t>(skvx::pin(px * 255.f + 0.5f, skvx::float4(0.f), skvx::float4(255.f)))
                       .store(&l32);
    return l32;
}
