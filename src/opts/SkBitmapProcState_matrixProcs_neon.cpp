/* Copyright 2009 Motorola
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkShader.h"
#include "SkUtilsArm.h"
#include "SkBitmapProcState_utils.h"

#include <arm_neon.h>

extern const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs_neon[];
extern const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs_neon[];

static void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count);
static void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count);

// TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
static inline int16x8_t sbpsm_clamp_tile8(int32x4_t low, int32x4_t high, unsigned max) {
    int16x8_t res;

    // get the hi 16s of all those 32s
    res = vuzpq_s16(vreinterpretq_s16_s32(low), vreinterpretq_s16_s32(high)).val[1];

    // clamp
    res = vmaxq_s16(res, vdupq_n_s16(0));
    res = vminq_s16(res, vdupq_n_s16(max));

    return res;
}

// TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
static inline int32x4_t sbpsm_clamp_tile4(int32x4_t f, unsigned max) {
    int32x4_t res;

    // get the hi 16s of all those 32s
    res = vshrq_n_s32(f, 16);

    // clamp
    res = vmaxq_s32(res, vdupq_n_s32(0));
    res = vminq_s32(res, vdupq_n_s32(max));

    return res;
}

// EXTRACT_LOW_BITS(fy, max)         (((fy) >> 12) & 0xF)
static inline int32x4_t sbpsm_clamp_tile4_low_bits(int32x4_t fx) {
    int32x4_t ret;

    ret = vshrq_n_s32(fx, 12);

    /* We don't need the mask below because the caller will
     * overwrite the non-masked bits
     */
    //ret = vandq_s32(ret, vdupq_n_s32(0xF));

    return ret;
}

// TILEX_PROCF(fx, max) (((fx)&0xFFFF)*((max)+1)>> 16)
static inline int16x8_t sbpsm_repeat_tile8(int32x4_t low, int32x4_t high, unsigned max) {
    uint16x8_t res;
    uint32x4_t tmpl, tmph;

    // get the lower 16 bits
    res = vuzpq_u16(vreinterpretq_u16_s32(low), vreinterpretq_u16_s32(high)).val[0];

    // bare multiplication, not SkFixedMul
    tmpl = vmull_u16(vget_low_u16(res), vdup_n_u16(max+1));
    tmph = vmull_u16(vget_high_u16(res), vdup_n_u16(max+1));

    // extraction of the 16 upper bits
    res = vuzpq_u16(vreinterpretq_u16_u32(tmpl), vreinterpretq_u16_u32(tmph)).val[1];

    return vreinterpretq_s16_u16(res);
}

// TILEX_PROCF(fx, max) (((fx)&0xFFFF)*((max)+1)>> 16)
static inline int32x4_t sbpsm_repeat_tile4(int32x4_t f, unsigned max) {
    uint16x4_t res;
    uint32x4_t tmp;

    // get the lower 16 bits
    res = vmovn_u32(vreinterpretq_u32_s32(f));

    // bare multiplication, not SkFixedMul
    tmp = vmull_u16(res, vdup_n_u16(max+1));

    // extraction of the 16 upper bits
    tmp = vshrq_n_u32(tmp, 16);

    return vreinterpretq_s32_u32(tmp);
}

// EXTRACT_LOW_BITS(fx, max)         ((((fx) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
static inline int32x4_t sbpsm_repeat_tile4_low_bits(int32x4_t fx, unsigned max) {
    uint16x4_t res;
    uint32x4_t tmp;
    int32x4_t ret;

    // get the lower 16 bits
    res = vmovn_u32(vreinterpretq_u32_s32(fx));

    // bare multiplication, not SkFixedMul
    tmp = vmull_u16(res, vdup_n_u16(max + 1));

    // shift and mask
    ret = vshrq_n_s32(vreinterpretq_s32_u32(tmp), 12);

    /* We don't need the mask below because the caller will
     * overwrite the non-masked bits
     */
    //ret = vandq_s32(ret, vdupq_n_s32(0xF));

    return ret;
}

#define MAKENAME(suffix)                ClampX_ClampY ## suffix ## _neon
#define TILEX_PROCF(fx, max)            SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)            SkClampMax((fy) >> 16, max)
#define TILEX_PROCF_NEON8(l, h, max)    sbpsm_clamp_tile8(l, h, max)
#define TILEY_PROCF_NEON8(l, h, max)    sbpsm_clamp_tile8(l, h, max)
#define TILEX_PROCF_NEON4(fx, max)      sbpsm_clamp_tile4(fx, max)
#define TILEY_PROCF_NEON4(fy, max)      sbpsm_clamp_tile4(fy, max)
#define EXTRACT_LOW_BITS(v, max)        (((v) >> 12) & 0xF)
#define EXTRACT_LOW_BITS_NEON4(v, max)  sbpsm_clamp_tile4_low_bits(v)
#define CHECK_FOR_DECAL
#include "SkBitmapProcState_matrix_neon.h"

#define MAKENAME(suffix)                RepeatX_RepeatY ## suffix ## _neon
#define TILEX_PROCF(fx, max)            SK_USHIFT16(((fx) & 0xFFFF) * ((max) + 1))
#define TILEY_PROCF(fy, max)            SK_USHIFT16(((fy) & 0xFFFF) * ((max) + 1))
#define TILEX_PROCF_NEON8(l, h, max)    sbpsm_repeat_tile8(l, h, max)
#define TILEY_PROCF_NEON8(l, h, max)    sbpsm_repeat_tile8(l, h, max)
#define TILEX_PROCF_NEON4(fx, max)      sbpsm_repeat_tile4(fx, max)
#define TILEY_PROCF_NEON4(fy, max)      sbpsm_repeat_tile4(fy, max)
#define EXTRACT_LOW_BITS(v, max)        ((((v) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#define EXTRACT_LOW_BITS_NEON4(v, max)  sbpsm_repeat_tile4_low_bits(v, max)
#include "SkBitmapProcState_matrix_neon.h"



void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    if (count >= 8) {
        // SkFixed is 16.16 fixed point
        SkFixed dx8 = dx * 8;
        int32x4_t vdx8 = vdupq_n_s32(dx8);

        // setup lbase and hbase
        int32x4_t lbase, hbase;
        lbase = vdupq_n_s32(fx);
        lbase = vsetq_lane_s32(fx + dx, lbase, 1);
        lbase = vsetq_lane_s32(fx + dx + dx, lbase, 2);
        lbase = vsetq_lane_s32(fx + dx + dx + dx, lbase, 3);
        hbase = lbase + vdupq_n_s32(4 * dx);

        do {
            // store the upper 16 bits
            vst1q_u32(dst, vreinterpretq_u32_s16(
                vuzpq_s16(vreinterpretq_s16_s32(lbase), vreinterpretq_s16_s32(hbase)).val[1]
            ));

            // on to the next group of 8
            lbase += vdx8;
            hbase += vdx8;
            dst += 4; // we did 8 elements but the result is twice smaller
            count -= 8;
            fx += dx8;
        } while (count >= 8);
    }

    uint16_t* xx = (uint16_t*)dst;
    for (int i = count; i > 0; --i) {
        *xx++ = SkToU16(fx >> 16); fx += dx;
    }
}

void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    if (count >= 8) {
        SkFixed dx8 = dx * 8;
        int32x4_t vdx8 = vdupq_n_s32(dx8);

        int32x4_t wide_fx, wide_fx2;
        wide_fx = vdupq_n_s32(fx);
        wide_fx = vsetq_lane_s32(fx + dx, wide_fx, 1);
        wide_fx = vsetq_lane_s32(fx + dx + dx, wide_fx, 2);
        wide_fx = vsetq_lane_s32(fx + dx + dx + dx, wide_fx, 3);

        wide_fx2 = vaddq_s32(wide_fx, vdupq_n_s32(4 * dx));

        while (count >= 8) {
            int32x4_t wide_out;
            int32x4_t wide_out2;

            wide_out = vshlq_n_s32(vshrq_n_s32(wide_fx, 12), 14);
            wide_out = wide_out | (vshrq_n_s32(wide_fx,16) + vdupq_n_s32(1));

            wide_out2 = vshlq_n_s32(vshrq_n_s32(wide_fx2, 12), 14);
            wide_out2 = wide_out2 | (vshrq_n_s32(wide_fx2,16) + vdupq_n_s32(1));

            vst1q_u32(dst, vreinterpretq_u32_s32(wide_out));
            vst1q_u32(dst+4, vreinterpretq_u32_s32(wide_out2));

            dst += 8;
            fx += dx8;
            wide_fx += vdx8;
            wide_fx2 += vdx8;
            count -= 8;
        }
    }

    if (count & 1)
    {
        SkASSERT((fx >> (16 + 14)) == 0);
        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;
    }
    while ((count -= 2) >= 0)
    {
        SkASSERT((fx >> (16 + 14)) == 0);
        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;

        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;
    }
}
