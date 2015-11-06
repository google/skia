/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_opts_DEFINED
#define SkBlitMask_opts_DEFINED

#include "Sk4px.h"

namespace SK_OPTS_NS {

#if defined(SK_ARM_HAS_NEON)
    // The Sk4px versions below will work fine with NEON, but we have had many indications
    // that it doesn't perform as well as this NEON-specific code.  TODO(mtklein): why?
    #include "SkColor_opts_neon.h"

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

    static void blit_mask_d32_a8_general(SkPMColor* dst, size_t dstRB,
                                         const SkAlpha* mask, size_t maskRB,
                                         SkColor color, int w, int h) {
        D32_A8_Opaque_Color_neon<true>(dst, dstRB, mask, maskRB, color, w, h);
    }

    // As above, but made slightly simpler by requiring that color is opaque.
    static void blit_mask_d32_a8_opaque(SkPMColor* dst, size_t dstRB,
                                        const SkAlpha* mask, size_t maskRB,
                                        SkColor color, int w, int h) {
        D32_A8_Opaque_Color_neon<false>(dst, dstRB, mask, maskRB, color, w, h);
    }

    // Same as _opaque, but assumes color == SK_ColorBLACK, a very common and even simpler case.
    static void blit_mask_d32_a8_black(SkPMColor* dst, size_t dstRB,
                                       const SkAlpha* maskPtr, size_t maskRB,
                                       int width, int height) {
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

#else
    static void blit_mask_d32_a8_general(SkPMColor* dst, size_t dstRB,
                                         const SkAlpha* mask, size_t maskRB,
                                         SkColor color, int w, int h) {
        auto s = Sk4px::DupPMColor(SkPreMultiplyColor(color));
        auto fn = [&](const Sk4px& d, const Sk4px& aa) {
            //  = (s + d(1-sa))aa + d(1-aa)
            //  = s*aa + d(1-sa*aa)
            auto left  = s.approxMulDiv255(aa),
                 right = d.approxMulDiv255(left.alphas().inv());
            return left + right;  // This does not overflow (exhaustively checked).
        };
        while (h --> 0) {
            Sk4px::MapDstAlpha(w, dst, mask, fn);
            dst  +=  dstRB / sizeof(*dst);
            mask += maskRB / sizeof(*mask);
        }
    }

    // As above, but made slightly simpler by requiring that color is opaque.
    static void blit_mask_d32_a8_opaque(SkPMColor* dst, size_t dstRB,
                                        const SkAlpha* mask, size_t maskRB,
                                        SkColor color, int w, int h) {
        SkASSERT(SkColorGetA(color) == 0xFF);
        auto s = Sk4px::DupPMColor(SkPreMultiplyColor(color));
        auto fn = [&](const Sk4px& d, const Sk4px& aa) {
            //  = (s + d(1-sa))aa + d(1-aa)
            //  = s*aa + d(1-sa*aa)
            //   ~~~>
            //  = s*aa + d(1-aa)
            return s.approxMulDiv255(aa) + d.approxMulDiv255(aa.inv());
        };
        while (h --> 0) {
            Sk4px::MapDstAlpha(w, dst, mask, fn);
            dst  +=  dstRB / sizeof(*dst);
            mask += maskRB / sizeof(*mask);
        }
    }

    // Same as _opaque, but assumes color == SK_ColorBLACK, a very common and even simpler case.
    static void blit_mask_d32_a8_black(SkPMColor* dst, size_t dstRB,
                                       const SkAlpha* mask, size_t maskRB,
                                       int w, int h) {
        auto fn = [](const Sk4px& d, const Sk4px& aa) {
            //   = (s + d(1-sa))aa + d(1-aa)
            //   = s*aa + d(1-sa*aa)
            //   ~~~>
            // a = 1*aa + d(1-1*aa) = aa + d(1-aa)
            // c = 0*aa + d(1-1*aa) =      d(1-aa)
            return aa.zeroColors() + d.approxMulDiv255(aa.inv());
        };
        while (h --> 0) {
            Sk4px::MapDstAlpha(w, dst, mask, fn);
            dst  +=  dstRB / sizeof(*dst);
            mask += maskRB / sizeof(*mask);
        }
    }
#endif

static void blit_mask_d32_a8(SkPMColor* dst, size_t dstRB,
                             const SkAlpha* mask, size_t maskRB,
                             SkColor color, int w, int h) {
    if (color == SK_ColorBLACK) {
        blit_mask_d32_a8_black(dst, dstRB, mask, maskRB, w, h);
    } else if (SkColorGetA(color) == 0xFF) {
        blit_mask_d32_a8_opaque(dst, dstRB, mask, maskRB, color, w, h);
    } else {
        blit_mask_d32_a8_general(dst, dstRB, mask, maskRB, color, w, h);
    }
}

}  // SK_OPTS_NS

#endif//SkBlitMask_opts_DEFINED
