/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitMask_opts_DEFINED
#define SkBlitMask_opts_DEFINED

#include "include/private/base/SkFeatures.h"
#include "src/core/Sk4px.h"

#if defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>
#endif

namespace SK_OPTS_NS {

#if defined(SK_ARM_HAS_NEON)
    // The Sk4px versions below will work fine with NEON, but we have had many indications
    // that it doesn't perform as well as this NEON-specific code.  TODO(mtklein): why?

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

        ret.val[0] = SkAlphaMul_neon8(color.val[0], scale);
        ret.val[1] = SkAlphaMul_neon8(color.val[1], scale);
        ret.val[2] = SkAlphaMul_neon8(color.val[2], scale);
        ret.val[3] = SkAlphaMul_neon8(color.val[3], scale);

        return ret;
    }


    template <bool isColor>
    static void D32_A8_Opaque_Color_neon(void* SK_RESTRICT dst, size_t dstRB,
                                         const void* SK_RESTRICT maskPtr, size_t maskRB,
                                         SkColor color, int width, int height) {
        SkPMColor pmc = SkPreMultiplyColor(color);
        SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
        const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;
        uint8x8x4_t vpmc;

        // Nine patch may set maskRB to 0 to blit the same row repeatedly.
        ptrdiff_t mask_adjust = (ptrdiff_t)maskRB - width;
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
            }

            device = (uint32_t*)((char*)device + dstRB);
            mask += mask_adjust;

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

        // Nine patch may set maskRB to 0 to blit the same row repeatedly.
        ptrdiff_t mask_adjust = (ptrdiff_t)maskRB - width;
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
            }
            device = (uint32_t*)((char*)device + dstRB);
            mask += mask_adjust;
        } while (--height != 0);
    }

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lsxintrin.h>

    static __m128i SkAlphaMul_lsx(__m128i x, __m128i y) {
        __m128i tmp = __lsx_vmul_h(x, y);
        __m128i mask = __lsx_vreplgr2vr_h(0xff00);
        return __lsx_vsrlri_h(__lsx_vand_v(tmp, mask), 8);
    }

    template <bool isColor>
    static void D32_A8_Opaque_Color_lsx(void* SK_RESTRICT dst, size_t dstRB,
                                         const void* SK_RESTRICT maskPtr, size_t maskRB,
                                         SkColor color, int width, int height) {
        SkPMColor pmc = SkPreMultiplyColor(color);
        SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
        const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;
        __m128i vpmc_b = __lsx_vldi(0);
        __m128i vpmc_g = __lsx_vldi(0);
        __m128i vpmc_r = __lsx_vldi(0);
        __m128i vpmc_a = __lsx_vldi(0);

        // Nine patch may set maskRB to 0 to blit the same row repeatedly.
        ptrdiff_t mask_adjust = (ptrdiff_t)maskRB - width;
        dstRB -= (width << 2);

        if (width >= 8) {
            vpmc_b = __lsx_vreplgr2vr_h(SkGetPackedB32(pmc));
            vpmc_g = __lsx_vreplgr2vr_h(SkGetPackedG32(pmc));
            vpmc_r = __lsx_vreplgr2vr_h(SkGetPackedR32(pmc));
            vpmc_a = __lsx_vreplgr2vr_h(SkGetPackedA32(pmc));
        }

        const __m128i zeros = __lsx_vldi(0);
        __m128i planar = __lsx_vldi(0);
        planar = __lsx_vinsgr2vr_d(planar, 0x0d0905010c080400, 0);
        planar = __lsx_vinsgr2vr_d(planar, 0x0f0b07030e0a0602, 1);

        do{
            int w = width;
            while(w >= 8){
                __m128i lo = __lsx_vld(device, 0);         // bgra bgra bgra bgra
                __m128i hi = __lsx_vld(device, 16);        // BGRA BGRA BGRA BGRA
                lo = __lsx_vshuf_b(zeros, lo, planar);     // bbbb gggg rrrr aaaa
                hi = __lsx_vshuf_b(zeros, hi, planar);     // BBBB GGGG RRRR AAAA
                __m128i bg = __lsx_vilvl_w(hi, lo),        // bbbb BBBB gggg GGGG
                        ra = __lsx_vilvh_w(hi, lo);        // rrrr RRRR aaaa AAAA

                __m128i b = __lsx_vilvl_b(zeros, bg),      // _b_b _b_b _B_B _B_B
                        g = __lsx_vilvh_b(zeros, bg),      // _g_g _g_g _G_G _G_G
                        r = __lsx_vilvl_b(zeros, ra),      // _r_r _r_r _R_R _R_R
                        a = __lsx_vilvh_b(zeros, ra);      // _a_a _a_a _A_A _A_A

                __m128i vmask = __lsx_vld(mask, 0);
                vmask = __lsx_vilvl_b(zeros, vmask);
                __m128i vscale, vmask256 = __lsx_vadd_h(vmask, __lsx_vreplgr2vr_h(1));

                if (isColor) {
                    __m128i tmp = SkAlphaMul_lsx(vpmc_a, vmask256);
                    vscale = __lsx_vsub_h(__lsx_vreplgr2vr_h(256), tmp);
                } else {
                    vscale = __lsx_vsub_h(__lsx_vreplgr2vr_h(256), vmask);
                }

                b = SkAlphaMul_lsx(vpmc_b, vmask256) + SkAlphaMul_lsx(b, vscale);
                g = SkAlphaMul_lsx(vpmc_g, vmask256) + SkAlphaMul_lsx(g, vscale);
                r = SkAlphaMul_lsx(vpmc_r, vmask256) + SkAlphaMul_lsx(r, vscale);
                a = SkAlphaMul_lsx(vpmc_a, vmask256) + SkAlphaMul_lsx(a, vscale);

                bg = __lsx_vor_v(b, __lsx_vslli_h(g, 8));  // bgbg bgbg BGBG BGBG
                ra = __lsx_vor_v(r, __lsx_vslli_h(a, 8));  // rara rara RARA RARA
                lo = __lsx_vilvl_h(ra, bg);                // bgra bgra bgra bgra
                hi = __lsx_vilvh_h(ra, bg);                // BGRA BGRA BGRA BGRA

                __lsx_vst(lo, device, 0);
                __lsx_vst(hi, device, 16);

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
            }

            device = (uint32_t *)((char*)device + dstRB);
            mask += mask_adjust;

        } while (--height != 0);
    }

    static void blit_mask_d32_a8_general(SkPMColor* dst, size_t dstRB,
                                         const SkAlpha* mask, size_t maskRB,
                                         SkColor color, int w, int h) {
        D32_A8_Opaque_Color_lsx<true>(dst, dstRB, mask, maskRB, color, w, h);
    }

    static void blit_mask_d32_a8_opaque(SkPMColor* dst, size_t dstRB,
                                         const SkAlpha* mask, size_t maskRB,
                                         SkColor color, int w, int h) {
        D32_A8_Opaque_Color_lsx<false>(dst, dstRB, mask, maskRB, color, w, h);
    }

    // Same as _opaque, but assumes color == SK_ColorBLACK, a very common and even simpler case.
    static void blit_mask_d32_a8_black(SkPMColor* dst, size_t dstRB,
                                       const SkAlpha* maskPtr, size_t maskRB,
                                       int width, int height) {
        SkPMColor* SK_RESTRICT device = (SkPMColor*)dst;
        const uint8_t* SK_RESTRICT mask = (const uint8_t*)maskPtr;

        // Nine patch may set maskRB to 0 to blit the same row repeatedly.
        ptrdiff_t mask_adjust = (ptrdiff_t)maskRB - width;
        dstRB -= (width << 2);
        const __m128i zeros = __lsx_vldi(0);
        __m128i planar = __lsx_vldi(0);
        planar = __lsx_vinsgr2vr_d(planar, 0x0d0905010c080400, 0);
        planar = __lsx_vinsgr2vr_d(planar, 0x0f0b07030e0a0602, 1);

        do {
            int w = width;
            while (w >= 8) {
                __m128i vmask = __lsx_vld(mask, 0);
                vmask = __lsx_vilvl_b(zeros, vmask);
                __m128i vscale = __lsx_vsub_h(__lsx_vreplgr2vr_h(256), vmask);
                __m128i lo = __lsx_vld(device, 0);         // bgra bgra bgra bgra
                __m128i hi = __lsx_vld(device, 16);        // BGRA BGRA BGRA BGRA
                lo = __lsx_vshuf_b(zeros, lo, planar);     // bbbb gggg rrrr aaaa
                hi = __lsx_vshuf_b(zeros, hi, planar);     // BBBB GGGG RRRR AAAA
                __m128i bg = __lsx_vilvl_w(hi, lo),        // bbbb BBBB gggg GGGG
                        ra = __lsx_vilvh_w(hi, lo);        // rrrr RRRR aaaa AAAA

                __m128i b = __lsx_vilvl_b(zeros, bg),      // _b_b _b_b _B_B _B_B
                        g = __lsx_vilvh_b(zeros, bg),      // _g_g _g_g _G_G _G_G
                        r = __lsx_vilvl_b(zeros, ra),      // _r_r _r_r _R_R _R_R
                        a = __lsx_vilvh_b(zeros, ra);      // _a_a _a_a _A_A _A_A

                b = SkAlphaMul_lsx(b, vscale);
                g = SkAlphaMul_lsx(g, vscale);
                r = SkAlphaMul_lsx(r, vscale);
                a = SkAlphaMul_lsx(a, vscale);

                a += vmask;

                bg = __lsx_vor_v(b, __lsx_vslli_h(g, 8));  // bgbg bgbg BGBG BGBG
                ra = __lsx_vor_v(r, __lsx_vslli_h(a, 8));  // rara rara RARA RARA
                lo = __lsx_vilvl_h(ra, bg);                // bgra bgra bgra bgra
                hi = __lsx_vilvh_h(ra, bg);                // BGRA BGRA BGRA BGRA

                __lsx_vst(lo, device, 0);
                __lsx_vst(hi, device, 16);

                mask += 8;
                device += 8;
                w -= 8;
            }

            while (w-- > 0) {
                unsigned aa = *mask++;
                *device = (aa << SK_A32_SHIFT)
                            + SkAlphaMulQ(*device, SkAlpha255To256(255 - aa));
                device += 1;
            }

            device = (uint32_t*)((char*)device + dstRB);
            mask += mask_adjust;

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
            return (aa & Sk4px(skvx::byte16{0,0,0,255, 0,0,0,255, 0,0,0,255, 0,0,0,255}))
                 + d.approxMulDiv255(aa.inv());
        };
        while (h --> 0) {
            Sk4px::MapDstAlpha(w, dst, mask, fn);
            dst  +=  dstRB / sizeof(*dst);
            mask += maskRB / sizeof(*mask);
        }
    }
#endif

/*not static*/ inline void blit_mask_d32_a8(SkPMColor* dst, size_t dstRB,
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

}  // namespace SK_OPTS_NS

#endif//SkBlitMask_opts_DEFINED
