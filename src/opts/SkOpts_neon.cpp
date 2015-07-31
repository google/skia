/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#include "SkFloatingPoint.h"

namespace neon {  // This helps identify methods from this file when debugging / profiling.

static float rsqrt(float x) {
    return sk_float_rsqrt(x);  // This sk_float_rsqrt copy will take the NEON compile-time path.
}

static void memset16(uint16_t* dst, uint16_t value, int n) {
    uint16x8_t   v8  = vdupq_n_u16(value);
    uint16x8x4_t v32 = {{ v8, v8, v8, v8 }};

    while (n >= 32) {
        vst4q_u16(dst, v32);  // This swizzles, but we don't care: all lanes are the same, value.
        dst += 32;
        n   -= 32;
    }
    switch (n / 8) {
        case 3: vst1q_u16(dst, v8); dst += 8;
        case 2: vst1q_u16(dst, v8); dst += 8;
        case 1: vst1q_u16(dst, v8); dst += 8;
    }
    if (n & 4) {
        vst1_u16(dst, vget_low_u16(v8));
        dst += 4;
    }
    switch (n & 3) {
        case 3: *dst++ = value;
        case 2: *dst++ = value;
        case 1: *dst   = value;
    }
}

static void memset32(uint32_t* dst, uint32_t value, int n) {
    uint32x4_t   v4  = vdupq_n_u32(value);
    uint32x4x4_t v16 = {{ v4, v4, v4, v4 }};

    while (n >= 16) {
        vst4q_u32(dst, v16);  // This swizzles, but we don't care: all lanes are the same, value.
        dst += 16;
        n   -= 16;
    }
    switch (n / 4) {
        case 3: vst1q_u32(dst, v4); dst += 4;
        case 2: vst1q_u32(dst, v4); dst += 4;
        case 1: vst1q_u32(dst, v4); dst += 4;
    }
    if (n & 2) {
        vst1_u32(dst, vget_low_u32(v4));
        dst += 2;
    }
    if (n & 1) {
        *dst = value;
    }
}

}  // namespace neon

namespace SkOpts {
    void Init_neon() {
        rsqrt    = neon::rsqrt;
        memset16 = neon::memset16;
        memset32 = neon::memset32;
    }
}
