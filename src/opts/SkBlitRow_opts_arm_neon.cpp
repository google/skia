/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow_opts_arm_neon.h"

#include "SkBlitMask.h"
#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMathPriv.h"
#include "SkUtils.h"

#include "SkColor_opts_neon.h"
#include <arm_neon.h>

/* Neon version of S32_Blend_BlitRow32()
 * portable version is in src/core/SkBlitRow_D32.cpp
 */
void S32_Blend_BlitRow32_neon(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);

    if (count <= 0) {
        return;
    }

    uint16_t src_scale = SkAlpha255To256(alpha);
    uint16_t dst_scale = 256 - src_scale;

    while (count >= 2) {
        uint8x8_t vsrc, vdst, vres;
        uint16x8_t vsrc_wide, vdst_wide;

        /* These commented prefetches are a big win for count
         * values > 64 on an A9 (Pandaboard) but hurt by 10% for count = 4.
         * They also hurt a little (<5%) on an A15
         */
        //__builtin_prefetch(src+32);
        //__builtin_prefetch(dst+32);

        // Load
        vsrc = vreinterpret_u8_u32(vld1_u32(src));
        vdst = vreinterpret_u8_u32(vld1_u32(dst));

        // Process src
        vsrc_wide = vmovl_u8(vsrc);
        vsrc_wide = vmulq_u16(vsrc_wide, vdupq_n_u16(src_scale));

        // Process dst
        vdst_wide = vmull_u8(vdst, vdup_n_u8(dst_scale));

        // Combine
        vdst_wide += vsrc_wide;
        vres = vshrn_n_u16(vdst_wide, 8);

        // Store
        vst1_u32(dst, vreinterpret_u32_u8(vres));

        src += 2;
        dst += 2;
        count -= 2;
    }

    if (count == 1) {
        uint8x8_t vsrc = vdup_n_u8(0), vdst = vdup_n_u8(0), vres;
        uint16x8_t vsrc_wide, vdst_wide;

        // Load
        vsrc = vreinterpret_u8_u32(vld1_lane_u32(src, vreinterpret_u32_u8(vsrc), 0));
        vdst = vreinterpret_u8_u32(vld1_lane_u32(dst, vreinterpret_u32_u8(vdst), 0));

        // Process
        vsrc_wide = vmovl_u8(vsrc);
        vsrc_wide = vmulq_u16(vsrc_wide, vdupq_n_u16(src_scale));
        vdst_wide = vmull_u8(vdst, vdup_n_u8(dst_scale));
        vdst_wide += vsrc_wide;
        vres = vshrn_n_u16(vdst_wide, 8);

        // Store
        vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);
    }
}

#ifdef SK_CPU_ARM32
void S32A_Blend_BlitRow32_neon(SkPMColor* SK_RESTRICT dst,
                         const SkPMColor* SK_RESTRICT src,
                         int count, U8CPU alpha) {

    SkASSERT(255 > alpha);

    if (count <= 0) {
        return;
    }

    unsigned alpha256 = SkAlpha255To256(alpha);

    // First deal with odd counts
    if (count & 1) {
        uint8x8_t vsrc = vdup_n_u8(0), vdst = vdup_n_u8(0), vres;
        uint16x8_t vdst_wide, vsrc_wide;
        unsigned dst_scale;

        // Load
        vsrc = vreinterpret_u8_u32(vld1_lane_u32(src, vreinterpret_u32_u8(vsrc), 0));
        vdst = vreinterpret_u8_u32(vld1_lane_u32(dst, vreinterpret_u32_u8(vdst), 0));

        // Calc dst_scale
        dst_scale = vget_lane_u8(vsrc, 3);
        dst_scale = SkAlphaMulInv256(dst_scale, alpha256);

        // Process src
        vsrc_wide = vmovl_u8(vsrc);
        vsrc_wide = vmulq_n_u16(vsrc_wide, alpha256);

        // Process dst
        vdst_wide = vmovl_u8(vdst);
        vdst_wide = vmulq_n_u16(vdst_wide, dst_scale);

        // Combine
        vdst_wide += vsrc_wide;
        vres = vshrn_n_u16(vdst_wide, 8);

        vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);
        dst++;
        src++;
        count--;
    }

    if (count) {
        uint8x8_t alpha_mask;
        static const uint8_t alpha_mask_setup[] = {3,3,3,3,7,7,7,7};
        alpha_mask = vld1_u8(alpha_mask_setup);

        do {

            uint8x8_t vsrc, vdst, vres, vsrc_alphas;
            uint16x8_t vdst_wide, vsrc_wide, vsrc_scale, vdst_scale;

            __builtin_prefetch(src+32);
            __builtin_prefetch(dst+32);

            // Load
            vsrc = vreinterpret_u8_u32(vld1_u32(src));
            vdst = vreinterpret_u8_u32(vld1_u32(dst));

            // Prepare src_scale
            vsrc_scale = vdupq_n_u16(alpha256);

            // Calc dst_scale
            vsrc_alphas = vtbl1_u8(vsrc, alpha_mask);
            vdst_scale = vmovl_u8(vsrc_alphas);
            // Calculate SkAlphaMulInv256(vdst_scale, vsrc_scale).
            // A 16-bit lane would overflow if we used 0xFFFF here,
            // so use an approximation with 0xFF00 that is off by 1,
            // and add back 1 after to get the correct value.
            // This is valid if alpha256 <= 255.
            vdst_scale = vmlsq_u16(vdupq_n_u16(0xFF00), vdst_scale, vsrc_scale);
            vdst_scale = vsraq_n_u16(vdst_scale, vdst_scale, 8);
            vdst_scale = vsraq_n_u16(vdupq_n_u16(1), vdst_scale, 8);

            // Process src
            vsrc_wide = vmovl_u8(vsrc);
            vsrc_wide *= vsrc_scale;

            // Process dst
            vdst_wide = vmovl_u8(vdst);
            vdst_wide *= vdst_scale;

            // Combine
            vdst_wide += vsrc_wide;
            vres = vshrn_n_u16(vdst_wide, 8);

            vst1_u32(dst, vreinterpret_u32_u8(vres));

            src += 2;
            dst += 2;
            count -= 2;
        } while(count);
    }
}

///////////////////////////////////////////////////////////////////////////////

#endif // #ifdef SK_CPU_ARM32

///////////////////////////////////////////////////////////////////////////////

const SkBlitRow::Proc32 sk_blitrow_platform_32_procs_arm_neon[] = {
    nullptr,   // S32_Opaque,
    S32_Blend_BlitRow32_neon,        // S32_Blend,
    nullptr,  // Ported to SkOpts
#ifdef SK_CPU_ARM32
    S32A_Blend_BlitRow32_neon        // S32A_Blend
#else
    nullptr
#endif
};
