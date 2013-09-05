/* NEON optimized code (C) COPYRIGHT 2009 Motorola
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkPerspIter.h"
#include "SkShader.h"
#include "SkUtilsArm.h"
#include "SkBitmapProcState_utils.h"

extern const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs_neon[];
extern const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs_neon[];

static void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count);
static void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count);

#define MAKENAME(suffix)        ClampX_ClampY ## suffix ## _neon
#define TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)    SkClampMax((fy) >> 16, max)
#define TILEX_LOW_BITS(fx, max) (((fx) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) (((fy) >> 12) & 0xF)
#define CHECK_FOR_DECAL
#include "SkBitmapProcState_matrix_clamp_neon.h"

#define MAKENAME(suffix)        RepeatX_RepeatY ## suffix ## _neon
#define TILEX_PROCF(fx, max)    SK_USHIFT16(((fx) & 0xFFFF) * ((max) + 1))
#define TILEY_PROCF(fy, max)    SK_USHIFT16(((fy) & 0xFFFF) * ((max) + 1))
#define TILEX_LOW_BITS(fx, max) ((((fx) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) ((((fy) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#include "SkBitmapProcState_matrix_repeat_neon.h"



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
