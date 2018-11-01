/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitMask.h"
#include "SkColor_opts_neon.h"

void SkBlitLCD16OpaqueRow_neon(SkPMColor dst[], const uint16_t src[],
                                        SkColor color, int width,
                                        SkPMColor opaqueDst) {
    int colR = SkColorGetR(color);
    int colG = SkColorGetG(color);
    int colB = SkColorGetB(color);

    uint8x8_t vcolR = vdup_n_u8(colR);
    uint8x8_t vcolG = vdup_n_u8(colG);
    uint8x8_t vcolB = vdup_n_u8(colB);
    uint8x8_t vopqDstA = vdup_n_u8(SkGetPackedA32(opaqueDst));
    uint8x8_t vopqDstR = vdup_n_u8(SkGetPackedR32(opaqueDst));
    uint8x8_t vopqDstG = vdup_n_u8(SkGetPackedG32(opaqueDst));
    uint8x8_t vopqDstB = vdup_n_u8(SkGetPackedB32(opaqueDst));

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

    uint16x8_t vcolA = vdupq_n_u16(colA);
    uint8x8_t vcolR = vdup_n_u8(colR);
    uint8x8_t vcolG = vdup_n_u8(colG);
    uint8x8_t vcolB = vdup_n_u8(colB);

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
