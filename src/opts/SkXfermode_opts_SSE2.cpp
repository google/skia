/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkColor_opts_SSE2.h"
#include "SkMathPriv.h"
#include "SkMath_opts_SSE2.h"
#include "SkXfermode.h"
#include "SkXfermode_opts_SSE2.h"
#include "SkXfermode_proccoeff.h"

////////////////////////////////////////////////////////////////////////////////
// 4 pixels SSE2 version functions
////////////////////////////////////////////////////////////////////////////////

static inline __m128i SkDiv255Round_SSE2(const __m128i& a) {
    __m128i prod = _mm_add_epi32(a, _mm_set1_epi32(128)); // prod += 128;
    prod = _mm_add_epi32(prod, _mm_srli_epi32(prod, 8));  // prod + (prod >> 8)
    prod = _mm_srli_epi32(prod, 8);                       // >> 8

    return prod;
}

static inline __m128i clamp_div255round_SSE2(const __m128i& prod) {
    // test if > 0
    __m128i cmp1 = _mm_cmpgt_epi32(prod, _mm_setzero_si128());
    // test if < 255*255
    __m128i cmp2 = _mm_cmplt_epi32(prod, _mm_set1_epi32(255*255));

    __m128i ret = _mm_setzero_si128();

    // if value >= 255*255, value = 255
    ret = _mm_andnot_si128(cmp2,  _mm_set1_epi32(255));

    __m128i div = SkDiv255Round_SSE2(prod);

    // test if > 0 && < 255*255
    __m128i cmp = _mm_and_si128(cmp1, cmp2);

    ret = _mm_or_si128(_mm_and_si128(cmp, div), _mm_andnot_si128(cmp, ret));

    return ret;
}
static inline __m128i SkMin32_SSE2(const __m128i& a, const __m128i& b) {
    __m128i cmp = _mm_cmplt_epi32(a, b);
    return _mm_or_si128(_mm_and_si128(cmp, a), _mm_andnot_si128(cmp, b));
}

static inline __m128i srcover_byte_SSE2(const __m128i& a, const __m128i& b) {
    // a + b - SkAlphaMulAlpha(a, b);
    return _mm_sub_epi32(_mm_add_epi32(a, b), SkAlphaMulAlpha_SSE2(a, b));

}

// Portable version overlay_byte() is in SkXfermode.cpp.
static inline __m128i overlay_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                        const __m128i& sa, const __m128i& da) {
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(255), da);
    __m128i tmp1 = _mm_mullo_epi16(sc, ida);
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(255), sa);
    __m128i tmp2 = _mm_mullo_epi16(dc, isa);
    __m128i tmp = _mm_add_epi32(tmp1, tmp2);

    __m128i cmp = _mm_cmpgt_epi32(_mm_slli_epi32(dc, 1), da);
    __m128i rc1 = _mm_slli_epi32(sc, 1);                        // 2 * sc
    rc1 = Multiply32_SSE2(rc1, dc);                             // *dc

    __m128i rc2 = _mm_mullo_epi16(sa, da);                      // sa * da
    __m128i tmp3 = _mm_slli_epi32(_mm_sub_epi32(da, dc), 1);    // 2 * (da - dc)
    tmp3 = Multiply32_SSE2(tmp3, _mm_sub_epi32(sa, sc));        // * (sa - sc)
    rc2 = _mm_sub_epi32(rc2, tmp3);

    __m128i rc = _mm_or_si128(_mm_andnot_si128(cmp, rc1),
                              _mm_and_si128(cmp, rc2));
    return clamp_div255round_SSE2(_mm_add_epi32(rc, tmp));
}

static __m128i overlay_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = overlay_byte_SSE2(SkGetPackedR32_SSE2(src),
                                  SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = overlay_byte_SSE2(SkGetPackedG32_SSE2(src),
                                  SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = overlay_byte_SSE2(SkGetPackedB32_SSE2(src),
                                  SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i darken_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                       const __m128i& sa, const __m128i& da) {
    __m128i sd = _mm_mullo_epi16(sc, da);
    __m128i ds = _mm_mullo_epi16(dc, sa);

    __m128i cmp = _mm_cmplt_epi32(sd, ds);

    __m128i tmp = _mm_add_epi32(sc, dc);
    __m128i ret1 = _mm_sub_epi32(tmp, SkDiv255Round_SSE2(ds));
    __m128i ret2 = _mm_sub_epi32(tmp, SkDiv255Round_SSE2(sd));
    __m128i ret = _mm_or_si128(_mm_and_si128(cmp, ret1),
                               _mm_andnot_si128(cmp, ret2));
    return ret;
}

static __m128i darken_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = darken_byte_SSE2(SkGetPackedR32_SSE2(src),
                                 SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = darken_byte_SSE2(SkGetPackedG32_SSE2(src),
                                 SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = darken_byte_SSE2(SkGetPackedB32_SSE2(src),
                                 SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i lighten_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                        const __m128i& sa, const __m128i& da) {
    __m128i sd = _mm_mullo_epi16(sc, da);
    __m128i ds = _mm_mullo_epi16(dc, sa);

    __m128i cmp = _mm_cmpgt_epi32(sd, ds);

    __m128i tmp = _mm_add_epi32(sc, dc);
    __m128i ret1 = _mm_sub_epi32(tmp, SkDiv255Round_SSE2(ds));
    __m128i ret2 = _mm_sub_epi32(tmp, SkDiv255Round_SSE2(sd));
    __m128i ret = _mm_or_si128(_mm_and_si128(cmp, ret1),
                               _mm_andnot_si128(cmp, ret2));
    return ret;
}

static __m128i lighten_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = lighten_byte_SSE2(SkGetPackedR32_SSE2(src),
                                  SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = lighten_byte_SSE2(SkGetPackedG32_SSE2(src),
                                  SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = lighten_byte_SSE2(SkGetPackedB32_SSE2(src),
                                  SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i colordodge_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                           const __m128i& sa, const __m128i& da) {
    __m128i diff = _mm_sub_epi32(sa, sc);
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(255), da);
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(255), sa);

    // if (0 == dc)
    __m128i cmp1 = _mm_cmpeq_epi32(dc, _mm_setzero_si128());
    __m128i rc1 = _mm_and_si128(cmp1, SkAlphaMulAlpha_SSE2(sc, ida));

    // else if (0 == diff)
    __m128i cmp2 = _mm_cmpeq_epi32(diff, _mm_setzero_si128());
    __m128i cmp = _mm_andnot_si128(cmp1, cmp2);
    __m128i tmp1 = _mm_mullo_epi16(sa, da);
    __m128i tmp2 = _mm_mullo_epi16(sc, ida);
    __m128i tmp3 = _mm_mullo_epi16(dc, isa);
    __m128i rc2 = _mm_add_epi32(tmp1, tmp2);
    rc2 = _mm_add_epi32(rc2, tmp3);
    rc2 = clamp_div255round_SSE2(rc2);
    rc2 = _mm_and_si128(cmp, rc2);

    // else
    __m128i cmp3 = _mm_or_si128(cmp1, cmp2);
    __m128i value = _mm_mullo_epi16(dc, sa);
    diff = shim_mm_div_epi32(value, diff);

    __m128i tmp4 = SkMin32_SSE2(da, diff);
    tmp4 = Multiply32_SSE2(sa, tmp4);
    __m128i rc3 = _mm_add_epi32(tmp4, tmp2);
    rc3 = _mm_add_epi32(rc3, tmp3);
    rc3 = clamp_div255round_SSE2(rc3);
    rc3 = _mm_andnot_si128(cmp3, rc3);

    __m128i rc = _mm_or_si128(rc1, rc2);
    rc = _mm_or_si128(rc, rc3);

    return rc;
}

static __m128i colordodge_modeproc_SSE2(const __m128i& src,
                                        const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = colordodge_byte_SSE2(SkGetPackedR32_SSE2(src),
                                     SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = colordodge_byte_SSE2(SkGetPackedG32_SSE2(src),
                                     SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = colordodge_byte_SSE2(SkGetPackedB32_SSE2(src),
                                     SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i colorburn_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                          const __m128i& sa, const __m128i& da) {
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(255), da);
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(255), sa);

    // if (dc == da)
    __m128i cmp1 = _mm_cmpeq_epi32(dc, da);
    __m128i tmp1 = _mm_mullo_epi16(sa, da);
    __m128i tmp2 = _mm_mullo_epi16(sc, ida);
    __m128i tmp3 = _mm_mullo_epi16(dc, isa);
    __m128i rc1 = _mm_add_epi32(tmp1, tmp2);
    rc1 = _mm_add_epi32(rc1, tmp3);
    rc1 = clamp_div255round_SSE2(rc1);
    rc1 = _mm_and_si128(cmp1, rc1);

    // else if (0 == sc)
    __m128i cmp2 = _mm_cmpeq_epi32(sc, _mm_setzero_si128());
    __m128i rc2 = SkAlphaMulAlpha_SSE2(dc, isa);
    __m128i cmp = _mm_andnot_si128(cmp1, cmp2);
    rc2 = _mm_and_si128(cmp, rc2);

    // else
    __m128i cmp3 = _mm_or_si128(cmp1, cmp2);
    __m128i tmp4 = _mm_sub_epi32(da, dc);
    tmp4 = Multiply32_SSE2(tmp4, sa);
    tmp4 = shim_mm_div_epi32(tmp4, sc);

    __m128i tmp5 = _mm_sub_epi32(da, SkMin32_SSE2(da, tmp4));
    tmp5 = Multiply32_SSE2(sa, tmp5);
    __m128i rc3 = _mm_add_epi32(tmp5, tmp2);
    rc3 = _mm_add_epi32(rc3, tmp3);
    rc3 = clamp_div255round_SSE2(rc3);
    rc3 = _mm_andnot_si128(cmp3, rc3);

    __m128i rc = _mm_or_si128(rc1, rc2);
    rc = _mm_or_si128(rc, rc3);

    return rc;
}

static __m128i colorburn_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = colorburn_byte_SSE2(SkGetPackedR32_SSE2(src),
                                    SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = colorburn_byte_SSE2(SkGetPackedG32_SSE2(src),
                                    SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = colorburn_byte_SSE2(SkGetPackedB32_SSE2(src),
                                    SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i hardlight_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                          const __m128i& sa, const __m128i& da) {
    // if (2 * sc <= sa)
    __m128i tmp1 = _mm_slli_epi32(sc, 1);
    __m128i cmp1 = _mm_cmpgt_epi32(tmp1, sa);
    __m128i rc1 = _mm_mullo_epi16(sc, dc);                // sc * dc;
    rc1 = _mm_slli_epi32(rc1, 1);                         // 2 * sc * dc
    rc1 = _mm_andnot_si128(cmp1, rc1);

    // else
    tmp1 = _mm_mullo_epi16(sa, da);
    __m128i tmp2 = Multiply32_SSE2(_mm_sub_epi32(da, dc),
                                   _mm_sub_epi32(sa, sc));
    tmp2 = _mm_slli_epi32(tmp2, 1);
    __m128i rc2 = _mm_sub_epi32(tmp1, tmp2);
    rc2 = _mm_and_si128(cmp1, rc2);

    __m128i rc = _mm_or_si128(rc1, rc2);

    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(255), da);
    tmp1 = _mm_mullo_epi16(sc, ida);
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(255), sa);
    tmp2 = _mm_mullo_epi16(dc, isa);
    rc = _mm_add_epi32(rc, tmp1);
    rc = _mm_add_epi32(rc, tmp2);
    return clamp_div255round_SSE2(rc);
}

static __m128i hardlight_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = hardlight_byte_SSE2(SkGetPackedR32_SSE2(src),
                                    SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = hardlight_byte_SSE2(SkGetPackedG32_SSE2(src),
                                    SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = hardlight_byte_SSE2(SkGetPackedB32_SSE2(src),
                                    SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static __m128i sqrt_unit_byte_SSE2(const __m128i& n) {
    return SkSqrtBits_SSE2(n, 15+4);
}

static inline __m128i softlight_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                          const __m128i& sa, const __m128i& da) {
    __m128i tmp1, tmp2, tmp3;

    // int m = da ? dc * 256 / da : 0;
    __m128i cmp = _mm_cmpeq_epi32(da, _mm_setzero_si128());
    __m128i m = _mm_slli_epi32(dc, 8);
    __m128 x = _mm_cvtepi32_ps(m);
    __m128 y = _mm_cvtepi32_ps(da);
    m = _mm_cvttps_epi32(_mm_div_ps(x, y));
    m = _mm_andnot_si128(cmp, m);

    // if (2 * sc <= sa)
    tmp1 = _mm_slli_epi32(sc, 1);                      // 2 * sc
    __m128i cmp1 = _mm_cmpgt_epi32(tmp1, sa);
    tmp1 = _mm_sub_epi32(tmp1, sa);                    // 2 * sc - sa
    tmp2 = _mm_sub_epi32(_mm_set1_epi32(256), m);      // 256 - m
    tmp1 = Multiply32_SSE2(tmp1, tmp2);
    tmp1 = _mm_srai_epi32(tmp1, 8);
    tmp1 = _mm_add_epi32(sa, tmp1);
    tmp1 = Multiply32_SSE2(dc, tmp1);
    __m128i rc1 = _mm_andnot_si128(cmp1, tmp1);

    // else if (4 * dc <= da)
    tmp2 = _mm_slli_epi32(dc, 2);                      // dc * 4
    __m128i cmp2 = _mm_cmpgt_epi32(tmp2, da);
    __m128i i = _mm_slli_epi32(m, 2);                  // 4 * m
    __m128i j = _mm_add_epi32(i, _mm_set1_epi32(256)); // 4 * m + 256
    __m128i k = Multiply32_SSE2(i, j);                 // 4 * m * (4 * m + 256)
    __m128i t = _mm_sub_epi32(m, _mm_set1_epi32(256)); // m - 256
    i = Multiply32_SSE2(k, t);                         // 4 * m * (4 * m + 256) * (m - 256)
    i = _mm_srai_epi32(i, 16);                         // >> 16
    j = Multiply32_SSE2(_mm_set1_epi32(7), m);         // 7 * m
    tmp2 = _mm_add_epi32(i, j);
    i = Multiply32_SSE2(dc, sa);                       // dc * sa
    j = _mm_slli_epi32(sc, 1);                         // 2 * sc
    j = _mm_sub_epi32(j, sa);                          // 2 * sc - sa
    j = Multiply32_SSE2(da, j);                        // da * (2 * sc - sa)
    tmp2 = Multiply32_SSE2(j, tmp2);                   // * tmp
    tmp2 = _mm_srai_epi32(tmp2, 8);                    // >> 8
    tmp2 = _mm_add_epi32(i, tmp2);
    cmp = _mm_andnot_si128(cmp2, cmp1);
    __m128i rc2 = _mm_and_si128(cmp, tmp2);
    __m128i rc = _mm_or_si128(rc1, rc2);

    // else
    tmp3 = sqrt_unit_byte_SSE2(m);
    tmp3 = _mm_sub_epi32(tmp3, m);
    tmp3 = Multiply32_SSE2(j, tmp3);                   // j = da * (2 * sc - sa)
    tmp3 = _mm_srai_epi32(tmp3, 8);
    tmp3 = _mm_add_epi32(i, tmp3);                     // i = dc * sa
    cmp = _mm_and_si128(cmp1, cmp2);
    __m128i rc3 = _mm_and_si128(cmp, tmp3);
    rc = _mm_or_si128(rc, rc3);

    tmp1 = _mm_sub_epi32(_mm_set1_epi32(255), da);     // 255 - da
    tmp1 = _mm_mullo_epi16(sc, tmp1);
    tmp2 = _mm_sub_epi32(_mm_set1_epi32(255), sa);     // 255 - sa
    tmp2 = _mm_mullo_epi16(dc, tmp2);
    rc = _mm_add_epi32(rc, tmp1);
    rc = _mm_add_epi32(rc, tmp2);
    return clamp_div255round_SSE2(rc);
}

static __m128i softlight_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = softlight_byte_SSE2(SkGetPackedR32_SSE2(src),
                                    SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = softlight_byte_SSE2(SkGetPackedG32_SSE2(src),
                                    SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = softlight_byte_SSE2(SkGetPackedB32_SSE2(src),
                                    SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}


////////////////////////////////////////////////////////////////////////////////

typedef __m128i (*SkXfermodeProcSIMD)(const __m128i& src, const __m128i& dst);

void SkSSE2ProcCoeffXfermode::xfer32(SkPMColor dst[], const SkPMColor src[],
                                     int count, const SkAlpha aa[]) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = this->getProc();
    SkXfermodeProcSIMD procSIMD = reinterpret_cast<SkXfermodeProcSIMD>(fProcSIMD);
    SkASSERT(procSIMD != NULL);

    if (NULL == aa) {
        if (count >= 4) {
            while (((size_t)dst & 0x0F) != 0) {
                *dst = proc(*src, *dst);
                dst++;
                src++;
                count--;
            }

            const __m128i* s = reinterpret_cast<const __m128i*>(src);
            __m128i* d = reinterpret_cast<__m128i*>(dst);

            while (count >= 4) {
                __m128i src_pixel = _mm_loadu_si128(s++);
                __m128i dst_pixel = _mm_load_si128(d);

                dst_pixel = procSIMD(src_pixel, dst_pixel);
                _mm_store_si128(d++, dst_pixel);
                count -= 4;
            }

            src = reinterpret_cast<const SkPMColor*>(s);
            dst = reinterpret_cast<SkPMColor*>(d);
        }

        for (int i = count - 1; i >= 0; --i) {
            *dst = proc(*src, *dst);
            dst++;
            src++;
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                SkPMColor dstC = dst[i];
                SkPMColor C = proc(src[i], dstC);
                if (a != 0xFF) {
                    C = SkFourByteInterp(C, dstC, a);
                }
                dst[i] = C;
            }
        }
    }
}

void SkSSE2ProcCoeffXfermode::xfer16(uint16_t dst[], const SkPMColor src[],
                                     int count, const SkAlpha aa[]) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = this->getProc();
    SkXfermodeProcSIMD procSIMD = reinterpret_cast<SkXfermodeProcSIMD>(fProcSIMD);
    SkASSERT(procSIMD != NULL);

    if (NULL == aa) {
        if (count >= 8) {
            while (((size_t)dst & 0x0F) != 0) {
                SkPMColor dstC = SkPixel16ToPixel32(*dst);
                *dst = SkPixel32ToPixel16_ToU16(proc(*src, dstC));
                dst++;
                src++;
                count--;
            }

            const __m128i* s = reinterpret_cast<const __m128i*>(src);
            __m128i* d = reinterpret_cast<__m128i*>(dst);

            while (count >= 8) {
                __m128i src_pixel1 = _mm_loadu_si128(s++);
                __m128i src_pixel2 = _mm_loadu_si128(s++);
                __m128i dst_pixel = _mm_load_si128(d);

                __m128i dst_pixel1 = _mm_unpacklo_epi16(dst_pixel, _mm_setzero_si128());
                __m128i dst_pixel2 = _mm_unpackhi_epi16(dst_pixel, _mm_setzero_si128());

                __m128i dstC1 = SkPixel16ToPixel32_SSE2(dst_pixel1);
                __m128i dstC2 = SkPixel16ToPixel32_SSE2(dst_pixel2);

                dst_pixel1 = procSIMD(src_pixel1, dstC1);
                dst_pixel2 = procSIMD(src_pixel2, dstC2);
                dst_pixel = SkPixel32ToPixel16_ToU16_SSE2(dst_pixel1, dst_pixel2);

                _mm_store_si128(d++, dst_pixel);
                count -= 8;
            }

            src = reinterpret_cast<const SkPMColor*>(s);
            dst = reinterpret_cast<uint16_t*>(d);
        }

        for (int i = count - 1; i >= 0; --i) {
            SkPMColor dstC = SkPixel16ToPixel32(*dst);
            *dst = SkPixel32ToPixel16_ToU16(proc(*src, dstC));
            dst++;
            src++;
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                SkPMColor C = proc(src[i], dstC);
                if (0xFF != a) {
                    C = SkFourByteInterp(C, dstC, a);
                }
                dst[i] = SkPixel32ToPixel16_ToU16(C);
            }
        }
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkSSE2ProcCoeffXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_SSE2(const ProcCoeff& rec,
                                                         SkXfermode::Mode mode) {
    SkXfermodeProcSIMD proc = nullptr;
    // TODO(mtklein): implement these Sk4px.
    switch (mode) {
        case SkProcCoeffXfermode::kOverlay_Mode:    proc =    overlay_modeproc_SSE2; break;
        case SkProcCoeffXfermode::kDarken_Mode:     proc =     darken_modeproc_SSE2; break;
        case SkProcCoeffXfermode::kLighten_Mode:    proc =    lighten_modeproc_SSE2; break;
        case SkProcCoeffXfermode::kColorDodge_Mode: proc = colordodge_modeproc_SSE2; break;
        case SkProcCoeffXfermode::kColorBurn_Mode:  proc =  colorburn_modeproc_SSE2; break;
        case SkProcCoeffXfermode::kHardLight_Mode:  proc =  hardlight_modeproc_SSE2; break;
        case SkProcCoeffXfermode::kSoftLight_Mode:  proc =  softlight_modeproc_SSE2; break;
        default: break;
    }
    return proc ? SkNEW_ARGS(SkSSE2ProcCoeffXfermode, (rec, mode, (void*)proc)) : nullptr;
}
