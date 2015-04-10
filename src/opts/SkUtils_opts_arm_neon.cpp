/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include <arm_neon.h>

void sk_memset32_neon(uint32_t dst[], uint32_t value, int count) {
    uint32x4_t   v4  = vdupq_n_u32(value);
    uint32x4x4_t v16 = { v4, v4, v4, v4 };

    while (count >= 16) {
        vst4q_u32(dst, v16);  // This swizzles, but we don't care: all lanes are the same, value.
        dst   += 16;
        count -= 16;
    }
    SkASSERT(count < 16);
    switch (count / 4) {
        case 3: vst1q_u32(dst, v4); dst += 4; count -= 4;
        case 2: vst1q_u32(dst, v4); dst += 4; count -= 4;
        case 1: vst1q_u32(dst, v4); dst += 4; count -= 4;
    }
    SkASSERT(count < 4);
    if (count >= 2) {
        vst1_u32(dst, vget_low_u32(v4));
        dst   += 2;
        count -= 2;
    }
    SkASSERT(count < 2);
    if (count > 0) {
        *dst = value;
    }
}

void sk_memset16_neon(uint16_t dst[], uint16_t value, int count) {
    uint16x8_t   v8  = vdupq_n_u16(value);
    uint16x8x4_t v32 = { v8, v8, v8, v8 };

    while (count >= 32) {
        vst4q_u16(dst, v32);  // This swizzles, but we don't care: all lanes are the same, value.
        dst   += 32;
        count -= 32;
    }
    SkASSERT(count < 32);
    switch (count / 8) {
        case 3: vst1q_u16(dst, v8); dst += 8; count -= 8;
        case 2: vst1q_u16(dst, v8); dst += 8; count -= 8;
        case 1: vst1q_u16(dst, v8); dst += 8; count -= 8;
    }
    SkASSERT(count < 8);
    if (count >= 4) {
        vst1_u16(dst, vget_low_u16(v8));
        dst   += 4;
        count -= 4;
    }
    SkASSERT(count < 4);
    switch (count) {
        case 3: *dst++ = value;
        case 2: *dst++ = value;
        case 1: *dst   = value;
    }
}

