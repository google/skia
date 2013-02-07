/* NEON optimized code (C) COPYRIGHT 2009 Motorola
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkPerspIter.h"
#include "SkShader.h"
#include "SkUtilsArm.h"

extern const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs_neon[];
extern const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs_neon[];

static void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count);
static void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count);

static unsigned SK_USHIFT16(unsigned x) {
    return x >> 16;
}

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


void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count)
{
    int i;

    if (count >= 8) {
        /* SkFixed is 16.16 fixed point */
        SkFixed dx2 = dx+dx;
        SkFixed dx4 = dx2+dx2;
        SkFixed dx8 = dx4+dx4;

        /* now build fx/fx+dx/fx+2dx/fx+3dx */
        SkFixed fx1, fx2, fx3;
        int32x4_t lbase, hbase;
        uint16_t *dst16 = (uint16_t *)dst;

        fx1 = fx+dx;
        fx2 = fx1+dx;
        fx3 = fx2+dx;

        /* avoid an 'lbase unitialized' warning */
        lbase = vdupq_n_s32(fx);
        lbase = vsetq_lane_s32(fx1, lbase, 1);
        lbase = vsetq_lane_s32(fx2, lbase, 2);
        lbase = vsetq_lane_s32(fx3, lbase, 3);
        hbase = vaddq_s32(lbase, vdupq_n_s32(dx4));

        /* take upper 16 of each, store, and bump everything */
        do {
            int32x4_t lout, hout;
            uint16x8_t hi16;

            lout = lbase;
            hout = hbase;
            /* gets hi's of all louts then hi's of all houts */
            asm ("vuzpq.16 %q0, %q1" : "+w" (lout), "+w" (hout));
            hi16 = vreinterpretq_u16_s32(hout);
            vst1q_u16(dst16, hi16);

            /* on to the next */
            lbase = vaddq_s32 (lbase, vdupq_n_s32(dx8));
            hbase = vaddq_s32 (hbase, vdupq_n_s32(dx8));
            dst16 += 8;
            count -= 8;
            fx += dx8;
        } while (count >= 8);
        dst = (uint32_t *) dst16;
    }

    uint16_t* xx = (uint16_t*)dst;
    for (i = count; i > 0; --i) {
        *xx++ = SkToU16(fx >> 16); fx += dx;
    }
}

void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count)
{
    if (count >= 8) {
        int32x4_t wide_fx;
        int32x4_t wide_fx2;
        int32x4_t wide_dx8 = vdupq_n_s32(dx*8);

        wide_fx = vdupq_n_s32(fx);
        wide_fx = vsetq_lane_s32(fx+dx, wide_fx, 1);
        wide_fx = vsetq_lane_s32(fx+dx+dx, wide_fx, 2);
        wide_fx = vsetq_lane_s32(fx+dx+dx+dx, wide_fx, 3);

        wide_fx2 = vaddq_s32(wide_fx, vdupq_n_s32(dx+dx+dx+dx));

        while (count >= 8) {
            int32x4_t wide_out;
            int32x4_t wide_out2;

            wide_out = vshlq_n_s32(vshrq_n_s32(wide_fx, 12), 14);
            wide_out = vorrq_s32(wide_out,
            vaddq_s32(vshrq_n_s32(wide_fx,16), vdupq_n_s32(1)));

            wide_out2 = vshlq_n_s32(vshrq_n_s32(wide_fx2, 12), 14);
            wide_out2 = vorrq_s32(wide_out2,
            vaddq_s32(vshrq_n_s32(wide_fx2,16), vdupq_n_s32(1)));

            vst1q_u32(dst, vreinterpretq_u32_s32(wide_out));
            vst1q_u32(dst+4, vreinterpretq_u32_s32(wide_out2));

            dst += 8;
            fx += dx*8;
            wide_fx = vaddq_s32(wide_fx, wide_dx8);
            wide_fx2 = vaddq_s32(wide_fx2, wide_dx8);
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
