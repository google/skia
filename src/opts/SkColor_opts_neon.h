/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColor_opts_neon_DEFINED
#define SkColor_opts_neon_DEFINED

#include "SkTypes.h"
#include "SkColorPriv.h"

#include <arm_neon.h>

#define NEON_A (SK_A32_SHIFT / 8)
#define NEON_R (SK_R32_SHIFT / 8)
#define NEON_G (SK_G32_SHIFT / 8)
#define NEON_B (SK_B32_SHIFT / 8)

static inline uint16x8_t SkAlpha255To256_neon8(uint8x8_t alpha) {
    return vaddw_u8(vdupq_n_u16(1), alpha);
}

static inline uint8x8_t SkAlphaMul_neon8(uint8x8_t color, uint16x8_t scale) {
    return vshrn_n_u16(vmovl_u8(color) * scale, 8);
}

static inline uint8x8x4_t SkAlphaMulQ_neon8(uint8x8x4_t color, uint16x8_t scale) {
    uint8x8x4_t ret;

    ret.val[NEON_A] = SkAlphaMul_neon8(color.val[NEON_A], scale);
    ret.val[NEON_R] = SkAlphaMul_neon8(color.val[NEON_R], scale);
    ret.val[NEON_G] = SkAlphaMul_neon8(color.val[NEON_G], scale);
    ret.val[NEON_B] = SkAlphaMul_neon8(color.val[NEON_B], scale);

    return ret;
}

/* This function expands 8 pixels from RGB565 (R, G, B from high to low) to
 * SkPMColor (all possible configurations supported) in the exact same way as
 * SkPixel16ToPixel32.
 */
static inline uint8x8x4_t SkPixel16ToPixel32_neon8(uint16x8_t vsrc) {

    uint8x8x4_t ret;
    uint8x8_t vr, vg, vb;

    vr = vmovn_u16(vshrq_n_u16(vsrc, SK_R16_SHIFT));
    vg = vmovn_u16(vshrq_n_u16(vshlq_n_u16(vsrc, SK_R16_BITS), SK_R16_BITS + SK_B16_BITS));
    vb = vmovn_u16(vsrc & vdupq_n_u16(SK_B16_MASK));

    ret.val[NEON_A] = vdup_n_u8(0xFF);
    ret.val[NEON_R] = vshl_n_u8(vr, 8 - SK_R16_BITS) | vshr_n_u8(vr, 2 * SK_R16_BITS - 8);
    ret.val[NEON_G] = vshl_n_u8(vg, 8 - SK_G16_BITS) | vshr_n_u8(vg, 2 * SK_G16_BITS - 8);
    ret.val[NEON_B] = vshl_n_u8(vb, 8 - SK_B16_BITS) | vshr_n_u8(vb, 2 * SK_B16_BITS - 8);

    return ret;
}

/* This function packs 8 pixels from SkPMColor (all possible configurations
 * supported) to RGB565 (R, G, B from high to low) in the exact same way as
 * SkPixel32ToPixel16.
 */
static inline uint16x8_t SkPixel32ToPixel16_neon8(uint8x8x4_t vsrc) {

    uint16x8_t ret;

    ret = vshll_n_u8(vsrc.val[NEON_R], 8);
    ret = vsriq_n_u16(ret, vshll_n_u8(vsrc.val[NEON_G], 8), SK_R16_BITS);
    ret = vsriq_n_u16(ret, vshll_n_u8(vsrc.val[NEON_B], 8), SK_R16_BITS + SK_G16_BITS);

    return ret;
}

/* This function blends 8 pixels of the same channel in the exact same way as
 * SkBlend32.
 */
static inline uint8x8_t SkBlend32_neon8(uint8x8_t src, uint8x8_t dst, uint16x8_t scale) {
    int16x8_t src_wide, dst_wide;

    src_wide = vreinterpretq_s16_u16(vmovl_u8(src));
    dst_wide = vreinterpretq_s16_u16(vmovl_u8(dst));

    src_wide = (src_wide - dst_wide) * vreinterpretq_s16_u16(scale);

    dst_wide += vshrq_n_s16(src_wide, 5);

    return vmovn_u16(vreinterpretq_u16_s16(dst_wide));
}

static inline SkPMColor SkFourByteInterp256_neon(SkPMColor src, SkPMColor dst,
                                                 unsigned srcScale) {
    SkASSERT(srcScale <= 256);
    int16x8_t vscale = vdupq_n_s16(srcScale);
    int16x8_t vsrc_wide, vdst_wide, vdiff;
    uint8x8_t res;

    vsrc_wide = vreinterpretq_s16_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(src))));
    vdst_wide = vreinterpretq_s16_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(dst))));

    vdiff = vsrc_wide - vdst_wide;
    vdiff *= vscale;

    vdiff = vshrq_n_s16(vdiff, 8);

    vdst_wide += vdiff;

    res = vmovn_u16(vreinterpretq_u16_s16(vdst_wide));

    return vget_lane_u32(vreinterpret_u32_u8(res), 0);
}

static inline SkPMColor SkFourByteInterp_neon(SkPMColor src, SkPMColor dst,
                                              U8CPU srcWeight) {
    SkASSERT(srcWeight <= 255);
    unsigned scale = SkAlpha255To256(srcWeight);
    return SkFourByteInterp256_neon(src, dst, scale);
}

#endif /* #ifndef SkColor_opts_neon_DEFINED */
