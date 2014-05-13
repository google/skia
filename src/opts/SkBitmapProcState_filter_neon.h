
/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include <arm_neon.h>
#include "SkColorPriv.h"

/*
 * Filter_32_opaque
 *
 * There is no hard-n-fast rule that the filtering must produce
 * exact results for the color components, but if the 4 incoming colors are
 * all opaque, then the output color must also be opaque. Subsequent parts of
 * the drawing pipeline may rely on this (e.g. which blitrow proc to use).
 *
 */
// Chrome on Android uses -Os so we need to force these inline. Otherwise
// calling the function in the inner loops will cause significant overhead on
// some platforms.
static SK_ALWAYS_INLINE void Filter_32_opaque_neon(unsigned x, unsigned y,
                                                   SkPMColor a00, SkPMColor a01,
                                                   SkPMColor a10, SkPMColor a11,
                                                   SkPMColor *dst) {
    uint8x8_t vy, vconst16_8, v16_y, vres;
    uint16x4_t vx, vconst16_16, v16_x, tmp;
    uint32x2_t va0, va1;
    uint16x8_t tmp1, tmp2;

    vy = vdup_n_u8(y);                // duplicate y into vy
    vconst16_8 = vmov_n_u8(16);       // set up constant in vconst16_8
    v16_y = vsub_u8(vconst16_8, vy);  // v16_y = 16-y

    va0 = vdup_n_u32(a00);            // duplicate a00
    va1 = vdup_n_u32(a10);            // duplicate a10
    va0 = vset_lane_u32(a01, va0, 1); // set top to a01
    va1 = vset_lane_u32(a11, va1, 1); // set top to a11

    tmp1 = vmull_u8(vreinterpret_u8_u32(va0), v16_y); // tmp1 = [a01|a00] * (16-y)
    tmp2 = vmull_u8(vreinterpret_u8_u32(va1), vy);    // tmp2 = [a11|a10] * y

    vx = vdup_n_u16(x);                // duplicate x into vx
    vconst16_16 = vmov_n_u16(16);      // set up constant in vconst16_16
    v16_x = vsub_u16(vconst16_16, vx); // v16_x = 16-x

    tmp = vmul_u16(vget_high_u16(tmp1), vx);        // tmp  = a01 * x
    tmp = vmla_u16(tmp, vget_high_u16(tmp2), vx);   // tmp += a11 * x
    tmp = vmla_u16(tmp, vget_low_u16(tmp1), v16_x); // tmp += a00 * (16-x)
    tmp = vmla_u16(tmp, vget_low_u16(tmp2), v16_x); // tmp += a10 * (16-x)

    vres = vshrn_n_u16(vcombine_u16(tmp, vcreate_u16(0)), 8); // shift down result by 8
    vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);         // store result
}

static SK_ALWAYS_INLINE void Filter_32_alpha_neon(unsigned x, unsigned y,
                                                  SkPMColor a00, SkPMColor a01,
                                                  SkPMColor a10, SkPMColor a11,
                                                  SkPMColor *dst,
                                                  uint16_t scale) {
    uint8x8_t vy, vconst16_8, v16_y, vres;
    uint16x4_t vx, vconst16_16, v16_x, tmp, vscale;
    uint32x2_t va0, va1;
    uint16x8_t tmp1, tmp2;

    vy = vdup_n_u8(y);                // duplicate y into vy
    vconst16_8 = vmov_n_u8(16);       // set up constant in vconst16_8
    v16_y = vsub_u8(vconst16_8, vy);  // v16_y = 16-y

    va0 = vdup_n_u32(a00);            // duplicate a00
    va1 = vdup_n_u32(a10);            // duplicate a10
    va0 = vset_lane_u32(a01, va0, 1); // set top to a01
    va1 = vset_lane_u32(a11, va1, 1); // set top to a11

    tmp1 = vmull_u8(vreinterpret_u8_u32(va0), v16_y); // tmp1 = [a01|a00] * (16-y)
    tmp2 = vmull_u8(vreinterpret_u8_u32(va1), vy);    // tmp2 = [a11|a10] * y

    vx = vdup_n_u16(x);                // duplicate x into vx
    vconst16_16 = vmov_n_u16(16);      // set up constant in vconst16_16
    v16_x = vsub_u16(vconst16_16, vx); // v16_x = 16-x

    tmp = vmul_u16(vget_high_u16(tmp1), vx);        // tmp  = a01 * x
    tmp = vmla_u16(tmp, vget_high_u16(tmp2), vx);   // tmp += a11 * x
    tmp = vmla_u16(tmp, vget_low_u16(tmp1), v16_x); // tmp += a00 * (16-x)
    tmp = vmla_u16(tmp, vget_low_u16(tmp2), v16_x); // tmp += a10 * (16-x)

    vscale = vdup_n_u16(scale);        // duplicate scale
    tmp = vshr_n_u16(tmp, 8);          // shift down result by 8
    tmp = vmul_u16(tmp, vscale);       // multiply result by scale

    vres = vshrn_n_u16(vcombine_u16(tmp, vcreate_u16(0)), 8); // shift down result by 8
    vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);         // store result
}
