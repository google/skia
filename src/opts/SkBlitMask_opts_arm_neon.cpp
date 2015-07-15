/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitMask.h"
#include "SkColor_opts_neon.h"

static void D32_A8_Black_neon(void* SK_RESTRICT dst, size_t dstRB,
                              const void* SK_RESTRICT maskPtr, size_t maskRB,
                              SkColor, int width, int height) {
    SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
    const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;

    maskRB -= width;
    dstRB -= (width << 2);
    do {
        int w = width;
        while (w >= 8) {
            uint8x8_t vmask = vld1_u8(mask);
            uint16x8_t vscale = vsubw_u8(vdupq_n_u16(256), vmask);
            uint8x8x4_t vdevice = vld4_u8((uint8_t*)device);

            vdevice = SkAlphaMulQ_neon8(vdevice, vscale);
            vdevice.val[NEON_A] += vmask;

            vst4_u8((uint8_t*)device, vdevice);

            mask += 8;
            device += 8;
            w -= 8;
        }
        while (w-- > 0) {
            unsigned aa = *mask++;
            *device = (aa << SK_A32_SHIFT)
                        + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
            device += 1;
        };
        device = (uint32_t*)((char*)device + dstRB);
        mask += maskRB;
    } while (--height != 0);
}

template <bool isColor>
static void D32_A8_Opaque_Color_neon(void* SK_RESTRICT dst, size_t dstRB,
                                     const void* SK_RESTRICT maskPtr, size_t maskRB,
                                     SkColor color, int width, int height) {
    SkPMColor pmc = SkPreMultiplyColor(color);
    SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
    const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;
    uint8x8x4_t vpmc;

    maskRB -= width;
    dstRB -= (width << 2);

    if (width >= 8) {
        vpmc.val[NEON_A] = vdup_n_u8(SkGetPackedA32(pmc));
        vpmc.val[NEON_R] = vdup_n_u8(SkGetPackedR32(pmc));
        vpmc.val[NEON_G] = vdup_n_u8(SkGetPackedG32(pmc));
        vpmc.val[NEON_B] = vdup_n_u8(SkGetPackedB32(pmc));
    }
    do {
        int w = width;
        while (w >= 8) {
            uint8x8_t vmask = vld1_u8(mask);
            uint16x8_t vscale, vmask256 = SkAlpha255To256_neon8(vmask);
            if (isColor) {
                vscale = vsubw_u8(vdupq_n_u16(256),
                            SkAlphaMul_neon8(vpmc.val[NEON_A], vmask256));
            } else {
                vscale = vsubw_u8(vdupq_n_u16(256), vmask);
            }
            uint8x8x4_t vdev = vld4_u8((uint8_t*)device);

            vdev.val[NEON_A] =   SkAlphaMul_neon8(vpmc.val[NEON_A], vmask256)
                               + SkAlphaMul_neon8(vdev.val[NEON_A], vscale);
            vdev.val[NEON_R] =   SkAlphaMul_neon8(vpmc.val[NEON_R], vmask256)
                               + SkAlphaMul_neon8(vdev.val[NEON_R], vscale);
            vdev.val[NEON_G] =   SkAlphaMul_neon8(vpmc.val[NEON_G], vmask256)
                               + SkAlphaMul_neon8(vdev.val[NEON_G], vscale);
            vdev.val[NEON_B] =   SkAlphaMul_neon8(vpmc.val[NEON_B], vmask256)
                               + SkAlphaMul_neon8(vdev.val[NEON_B], vscale);

            vst4_u8((uint8_t*)device, vdev);

            mask += 8;
            device += 8;
            w -= 8;
        }

        while (w--) {
            unsigned aa = *mask++;
            if (isColor) {
                *device = SkBlendARGB32(pmc, *device, aa);
            } else {
                *device = SkAlphaMulQ(pmc, SkAlpha255To256(aa))
                            + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
            }
            device += 1;
        };

        device = (uint32_t*)((char*)device + dstRB);
        mask += maskRB;

    } while (--height != 0);
}

static void D32_A8_Opaque_neon(void* SK_RESTRICT dst, size_t dstRB,
                               const void* SK_RESTRICT maskPtr, size_t maskRB,
                               SkColor color, int width, int height) {
    D32_A8_Opaque_Color_neon<false>(dst, dstRB, maskPtr, maskRB, color, width, height);
}

static void D32_A8_Color_neon(void* SK_RESTRICT dst, size_t dstRB,
                              const void* SK_RESTRICT maskPtr, size_t maskRB,
                              SkColor color, int width, int height) {
    D32_A8_Opaque_Color_neon<true>(dst, dstRB, maskPtr, maskRB, color, width, height);
}

SkBlitMask::ColorProc D32_A8_Factory_neon(SkColor color) {
    if (SK_ColorBLACK == color) {
        return D32_A8_Black_neon;
    } else if (0xFF == SkColorGetA(color)) {
        return D32_A8_Opaque_neon;
    } else {
        return D32_A8_Color_neon;
    }
}

////////////////////////////////////////////////////////////////////////////////

void SkBlitLCD16OpaqueRow_neon(SkPMColor dst[], const uint16_t src[],
                                        SkColor color, int width,
                                        SkPMColor opaqueDst) {
    int colR = SkColorGetR(color);
    int colG = SkColorGetG(color);
    int colB = SkColorGetB(color);

    uint8x8_t vcolR, vcolG, vcolB;
    uint8x8_t vopqDstA, vopqDstR, vopqDstG, vopqDstB;

    if (width >= 8) {
        vcolR = vdup_n_u8(colR);
        vcolG = vdup_n_u8(colG);
        vcolB = vdup_n_u8(colB);
        vopqDstA = vdup_n_u8(SkGetPackedA32(opaqueDst));
        vopqDstR = vdup_n_u8(SkGetPackedR32(opaqueDst));
        vopqDstG = vdup_n_u8(SkGetPackedG32(opaqueDst));
        vopqDstB = vdup_n_u8(SkGetPackedB32(opaqueDst));
    }

    while (width >= 8) {
        uint8x8x4_t vdst;
        uint16x8_t vmask;
        uint16x8_t vmaskR, vmaskG, vmaskB;
        uint8x8_t vsel_trans, vsel_opq;

        vdst = vld4_u8((uint8_t*)dst);
        vmask = vld1q_u16(src);

        // Prepare compare masks
        vsel_trans = vmovn_u16(vceqq_u16(vmask, vdupq_n_u16(0)));
        vsel_opq = vmovn_u16(vceqq_u16(vmask, vdupq_n_u16(0xFFFF)));

        // Get all the color masks on 5 bits
        vmaskR = vshrq_n_u16(vmask, SK_R16_SHIFT);
        vmaskG = vshrq_n_u16(vshlq_n_u16(vmask, SK_R16_BITS),
                             SK_B16_BITS + SK_R16_BITS + 1);
        vmaskB = vmask & vdupq_n_u16(SK_B16_MASK);

        // Upscale to 0..32
        vmaskR = vmaskR + vshrq_n_u16(vmaskR, 4);
        vmaskG = vmaskG + vshrq_n_u16(vmaskG, 4);
        vmaskB = vmaskB + vshrq_n_u16(vmaskB, 4);

        vdst.val[NEON_A] = vbsl_u8(vsel_trans, vdst.val[NEON_A], vdup_n_u8(0xFF));
        vdst.val[NEON_A] = vbsl_u8(vsel_opq, vopqDstA, vdst.val[NEON_A]);

        vdst.val[NEON_R] = SkBlend32_neon8(vcolR, vdst.val[NEON_R], vmaskR);
        vdst.val[NEON_G] = SkBlend32_neon8(vcolG, vdst.val[NEON_G], vmaskG);
        vdst.val[NEON_B] = SkBlend32_neon8(vcolB, vdst.val[NEON_B], vmaskB);

        vdst.val[NEON_R] = vbsl_u8(vsel_opq, vopqDstR, vdst.val[NEON_R]);
        vdst.val[NEON_G] = vbsl_u8(vsel_opq, vopqDstG, vdst.val[NEON_G]);
        vdst.val[NEON_B] = vbsl_u8(vsel_opq, vopqDstB, vdst.val[NEON_B]);

        vst4_u8((uint8_t*)dst, vdst);

        dst += 8;
        src += 8;
        width -= 8;
    }

    // Leftovers
    for (int i = 0; i < width; i++) {
        dst[i] = SkBlendLCD16Opaque(colR, colG, colB, dst[i], src[i],
                                    opaqueDst);
    }
}

void SkBlitLCD16Row_neon(SkPMColor dst[], const uint16_t src[],
                                   SkColor color, int width, SkPMColor) {
    int colA = SkColorGetA(color);
    int colR = SkColorGetR(color);
    int colG = SkColorGetG(color);
    int colB = SkColorGetB(color);

    colA = SkAlpha255To256(colA);

    uint8x8_t vcolR, vcolG, vcolB;
    uint16x8_t vcolA;

    if (width >= 8) {
        vcolA = vdupq_n_u16(colA);
        vcolR = vdup_n_u8(colR);
        vcolG = vdup_n_u8(colG);
        vcolB = vdup_n_u8(colB);
    }

    while (width >= 8) {
        uint8x8x4_t vdst;
        uint16x8_t vmask;
        uint16x8_t vmaskR, vmaskG, vmaskB;

        vdst = vld4_u8((uint8_t*)dst);
        vmask = vld1q_u16(src);

        // Get all the color masks on 5 bits
        vmaskR = vshrq_n_u16(vmask, SK_R16_SHIFT);
        vmaskG = vshrq_n_u16(vshlq_n_u16(vmask, SK_R16_BITS),
                             SK_B16_BITS + SK_R16_BITS + 1);
        vmaskB = vmask & vdupq_n_u16(SK_B16_MASK);

        // Upscale to 0..32
        vmaskR = vmaskR + vshrq_n_u16(vmaskR, 4);
        vmaskG = vmaskG + vshrq_n_u16(vmaskG, 4);
        vmaskB = vmaskB + vshrq_n_u16(vmaskB, 4);

        vmaskR = vshrq_n_u16(vmaskR * vcolA, 8);
        vmaskG = vshrq_n_u16(vmaskG * vcolA, 8);
        vmaskB = vshrq_n_u16(vmaskB * vcolA, 8);

        vdst.val[NEON_A] = vdup_n_u8(0xFF);
        vdst.val[NEON_R] = SkBlend32_neon8(vcolR, vdst.val[NEON_R], vmaskR);
        vdst.val[NEON_G] = SkBlend32_neon8(vcolG, vdst.val[NEON_G], vmaskG);
        vdst.val[NEON_B] = SkBlend32_neon8(vcolB, vdst.val[NEON_B], vmaskB);

        vst4_u8((uint8_t*)dst, vdst);

        dst += 8;
        src += 8;
        width -= 8;
    }

    for (int i = 0; i < width; i++) {
        dst[i] = SkBlendLCD16(colA, colR, colG, colB, dst[i], src[i]);
    }
}

#define LOAD_LANE_16(reg, n) \
    reg = vld1q_lane_u16(device, reg, n); \
    device = (uint16_t*)((char*)device + deviceRB);

#define STORE_LANE_16(reg, n) \
    vst1_lane_u16(dst, reg, n); \
    dst = (uint16_t*)((char*)dst + deviceRB);

void SkRGB16BlitterBlitV_neon(uint16_t* device,
                              int height,
                              size_t deviceRB,
                              unsigned scale,
                              uint32_t src32) {
    if (height >= 8)
    {
        uint16_t* dst = device;

        // prepare constants
        uint16x8_t vdev = vdupq_n_u16(0);
        uint16x8_t vmaskq_g16 = vdupq_n_u16(SK_G16_MASK_IN_PLACE);
        uint16x8_t vmaskq_ng16 = vdupq_n_u16(~SK_G16_MASK_IN_PLACE);
        uint32x4_t vsrc32 = vdupq_n_u32(src32);
        uint32x4_t vscale5 = vdupq_n_u32((uint32_t)scale);

        while (height >= 8){
            LOAD_LANE_16(vdev, 0)
            LOAD_LANE_16(vdev, 1)
            LOAD_LANE_16(vdev, 2)
            LOAD_LANE_16(vdev, 3)
            LOAD_LANE_16(vdev, 4)
            LOAD_LANE_16(vdev, 5)
            LOAD_LANE_16(vdev, 6)
            LOAD_LANE_16(vdev, 7)

            // Expand_rgb_16
            uint16x8x2_t vdst = vzipq_u16((vdev & vmaskq_ng16), (vdev & vmaskq_g16));
            uint32x4_t vdst32_lo = vmulq_u32(vreinterpretq_u32_u16(vdst.val[0]), vscale5);
            uint32x4_t vdst32_hi = vmulq_u32(vreinterpretq_u32_u16(vdst.val[1]), vscale5);

            // Compact_rgb_16
            vdst32_lo = vaddq_u32(vdst32_lo, vsrc32);
            vdst32_hi = vaddq_u32(vdst32_hi, vsrc32);
            vdst32_lo = vshrq_n_u32(vdst32_lo, 5);
            vdst32_hi = vshrq_n_u32(vdst32_hi, 5);

            uint16x4_t vtmp_lo = vmovn_u32(vdst32_lo) & vget_low_u16(vmaskq_ng16);
            uint16x4_t vtmp_hi = vshrn_n_u32(vdst32_lo, 16) & vget_low_u16(vmaskq_g16);
            uint16x4_t vdst16_lo = vorr_u16(vtmp_lo, vtmp_hi);
            vtmp_lo = vmovn_u32(vdst32_hi) & vget_low_u16(vmaskq_ng16);
            vtmp_hi = vshrn_n_u32(vdst32_hi, 16) & vget_low_u16(vmaskq_g16);
            uint16x4_t vdst16_hi = vorr_u16(vtmp_lo, vtmp_hi);

            STORE_LANE_16(vdst16_lo, 0)
            STORE_LANE_16(vdst16_lo, 1)
            STORE_LANE_16(vdst16_lo, 2)
            STORE_LANE_16(vdst16_lo, 3)
            STORE_LANE_16(vdst16_hi, 0)
            STORE_LANE_16(vdst16_hi, 1)
            STORE_LANE_16(vdst16_hi, 2)
            STORE_LANE_16(vdst16_hi, 3)
            height -= 8;
        }
    }
    while (height != 0){
        uint32_t dst32 = SkExpand_rgb_16(*device) * scale;
        *device = SkCompact_rgb_16((src32 + dst32) >> 5);
        device = (uint16_t*)((char*)device + deviceRB);
        height--;
    }
}

#undef LOAD_LANE_16
#undef STORE_LANE_16
