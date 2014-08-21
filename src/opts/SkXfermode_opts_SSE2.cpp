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

static inline __m128i saturated_add_SSE2(const __m128i& a, const __m128i& b) {
    __m128i sum = _mm_add_epi32(a, b);
    __m128i cmp = _mm_cmpgt_epi32(sum, _mm_set1_epi32(255));

    sum = _mm_or_si128(_mm_and_si128(cmp, _mm_set1_epi32(255)),
                       _mm_andnot_si128(cmp, sum));
    return sum;
}

static inline __m128i clamp_signed_byte_SSE2(const __m128i& n) {
    __m128i cmp1 = _mm_cmplt_epi32(n, _mm_setzero_si128());
    __m128i cmp2 = _mm_cmpgt_epi32(n, _mm_set1_epi32(255));
    __m128i ret = _mm_and_si128(cmp2, _mm_set1_epi32(255));

    __m128i cmp = _mm_or_si128(cmp1, cmp2);
    ret = _mm_or_si128(_mm_and_si128(cmp, ret), _mm_andnot_si128(cmp, n));

    return ret;
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

static __m128i srcover_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(256), SkGetPackedA32_SSE2(src));
    return _mm_add_epi32(src, SkAlphaMulQ_SSE2(dst, isa));
}

static __m128i dstover_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(256), SkGetPackedA32_SSE2(dst));
    return _mm_add_epi32(dst, SkAlphaMulQ_SSE2(src, ida));
}

static __m128i srcin_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i da = SkGetPackedA32_SSE2(dst);
    return SkAlphaMulQ_SSE2(src, SkAlpha255To256_SSE2(da));
}

static __m128i dstin_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    return SkAlphaMulQ_SSE2(dst, SkAlpha255To256_SSE2(sa));
}

static __m128i srcout_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(256), SkGetPackedA32_SSE2(dst));
    return SkAlphaMulQ_SSE2(src, ida);
}

static __m128i dstout_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(256), SkGetPackedA32_SSE2(src));
    return SkAlphaMulQ_SSE2(dst, isa);
}

static __m128i srcatop_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(255), sa);

    __m128i a = da;

    __m128i r1 = SkAlphaMulAlpha_SSE2(da, SkGetPackedR32_SSE2(src));
    __m128i r2 = SkAlphaMulAlpha_SSE2(isa, SkGetPackedR32_SSE2(dst));
    __m128i r = _mm_add_epi32(r1, r2);

    __m128i g1 = SkAlphaMulAlpha_SSE2(da, SkGetPackedG32_SSE2(src));
    __m128i g2 = SkAlphaMulAlpha_SSE2(isa, SkGetPackedG32_SSE2(dst));
    __m128i g = _mm_add_epi32(g1, g2);

    __m128i b1 = SkAlphaMulAlpha_SSE2(da, SkGetPackedB32_SSE2(src));
    __m128i b2 = SkAlphaMulAlpha_SSE2(isa, SkGetPackedB32_SSE2(dst));
    __m128i b = _mm_add_epi32(b1, b2);

    return SkPackARGB32_SSE2(a, r, g, b);
}

static __m128i dstatop_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(255), da);

    __m128i a = sa;

    __m128i r1 = SkAlphaMulAlpha_SSE2(ida, SkGetPackedR32_SSE2(src));
    __m128i r2 = SkAlphaMulAlpha_SSE2(sa, SkGetPackedR32_SSE2(dst));
    __m128i r = _mm_add_epi32(r1, r2);

    __m128i g1 = SkAlphaMulAlpha_SSE2(ida, SkGetPackedG32_SSE2(src));
    __m128i g2 = SkAlphaMulAlpha_SSE2(sa, SkGetPackedG32_SSE2(dst));
    __m128i g = _mm_add_epi32(g1, g2);

    __m128i b1 = SkAlphaMulAlpha_SSE2(ida, SkGetPackedB32_SSE2(src));
    __m128i b2 = SkAlphaMulAlpha_SSE2(sa, SkGetPackedB32_SSE2(dst));
    __m128i b = _mm_add_epi32(b1, b2);

    return SkPackARGB32_SSE2(a, r, g, b);
}

static __m128i xor_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);
    __m128i isa = _mm_sub_epi32(_mm_set1_epi32(255), sa);
    __m128i ida = _mm_sub_epi32(_mm_set1_epi32(255), da);

    __m128i a1 = _mm_add_epi32(sa, da);
    __m128i a2 = SkAlphaMulAlpha_SSE2(sa, da);
    a2 = _mm_slli_epi32(a2, 1);
    __m128i a = _mm_sub_epi32(a1, a2);

    __m128i r1 = SkAlphaMulAlpha_SSE2(ida, SkGetPackedR32_SSE2(src));
    __m128i r2 = SkAlphaMulAlpha_SSE2(isa, SkGetPackedR32_SSE2(dst));
    __m128i r = _mm_add_epi32(r1, r2);

    __m128i g1 = SkAlphaMulAlpha_SSE2(ida, SkGetPackedG32_SSE2(src));
    __m128i g2 = SkAlphaMulAlpha_SSE2(isa, SkGetPackedG32_SSE2(dst));
    __m128i g = _mm_add_epi32(g1, g2);

    __m128i b1 = SkAlphaMulAlpha_SSE2(ida, SkGetPackedB32_SSE2(src));
    __m128i b2 = SkAlphaMulAlpha_SSE2(isa, SkGetPackedB32_SSE2(dst));
    __m128i b = _mm_add_epi32(b1, b2);

    return SkPackARGB32_SSE2(a, r, g, b);
}

static __m128i plus_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i b = saturated_add_SSE2(SkGetPackedB32_SSE2(src),
                                   SkGetPackedB32_SSE2(dst));
    __m128i g = saturated_add_SSE2(SkGetPackedG32_SSE2(src),
                                   SkGetPackedG32_SSE2(dst));
    __m128i r = saturated_add_SSE2(SkGetPackedR32_SSE2(src),
                                   SkGetPackedR32_SSE2(dst));
    __m128i a = saturated_add_SSE2(SkGetPackedA32_SSE2(src),
                                   SkGetPackedA32_SSE2(dst));
    return SkPackARGB32_SSE2(a, r, g, b);
}

static __m128i modulate_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i a = SkAlphaMulAlpha_SSE2(SkGetPackedA32_SSE2(src),
                                     SkGetPackedA32_SSE2(dst));
    __m128i r = SkAlphaMulAlpha_SSE2(SkGetPackedR32_SSE2(src),
                                     SkGetPackedR32_SSE2(dst));
    __m128i g = SkAlphaMulAlpha_SSE2(SkGetPackedG32_SSE2(src),
                                     SkGetPackedG32_SSE2(dst));
    __m128i b = SkAlphaMulAlpha_SSE2(SkGetPackedB32_SSE2(src),
                                     SkGetPackedB32_SSE2(dst));
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i SkMin32_SSE2(const __m128i& a, const __m128i& b) {
    __m128i cmp = _mm_cmplt_epi32(a, b);
    return _mm_or_si128(_mm_and_si128(cmp, a), _mm_andnot_si128(cmp, b));
}

static inline __m128i srcover_byte_SSE2(const __m128i& a, const __m128i& b) {
    // a + b - SkAlphaMulAlpha(a, b);
    return _mm_sub_epi32(_mm_add_epi32(a, b), SkAlphaMulAlpha_SSE2(a, b));

}

static inline __m128i blendfunc_multiply_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                                   const __m128i& sa, const __m128i& da) {
    // sc * (255 - da)
    __m128i ret1 = _mm_sub_epi32(_mm_set1_epi32(255), da);
    ret1 = _mm_mullo_epi16(sc, ret1);

    // dc * (255 - sa)
    __m128i ret2 = _mm_sub_epi32(_mm_set1_epi32(255), sa);
    ret2 = _mm_mullo_epi16(dc, ret2);

    // sc * dc
    __m128i ret3 = _mm_mullo_epi16(sc, dc);

    __m128i ret = _mm_add_epi32(ret1, ret2);
    ret = _mm_add_epi32(ret, ret3);

    return clamp_div255round_SSE2(ret);
}

static __m128i multiply_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);
    __m128i a = srcover_byte_SSE2(sa, da);

    __m128i sr = SkGetPackedR32_SSE2(src);
    __m128i dr = SkGetPackedR32_SSE2(dst);
    __m128i r = blendfunc_multiply_byte_SSE2(sr, dr, sa, da);

    __m128i sg = SkGetPackedG32_SSE2(src);
    __m128i dg = SkGetPackedG32_SSE2(dst);
    __m128i g = blendfunc_multiply_byte_SSE2(sg, dg, sa, da);


    __m128i sb = SkGetPackedB32_SSE2(src);
    __m128i db = SkGetPackedB32_SSE2(dst);
    __m128i b = blendfunc_multiply_byte_SSE2(sb, db, sa, da);

    return SkPackARGB32_SSE2(a, r, g, b);
}

static __m128i screen_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i a = srcover_byte_SSE2(SkGetPackedA32_SSE2(src),
                                  SkGetPackedA32_SSE2(dst));
    __m128i r = srcover_byte_SSE2(SkGetPackedR32_SSE2(src),
                                  SkGetPackedR32_SSE2(dst));
    __m128i g = srcover_byte_SSE2(SkGetPackedG32_SSE2(src),
                                  SkGetPackedG32_SSE2(dst));
    __m128i b = srcover_byte_SSE2(SkGetPackedB32_SSE2(src),
                                  SkGetPackedB32_SSE2(dst));
    return SkPackARGB32_SSE2(a, r, g, b);
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

static inline __m128i difference_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                           const __m128i& sa, const __m128i& da) {
    __m128i tmp1 = _mm_mullo_epi16(sc, da);
    __m128i tmp2 = _mm_mullo_epi16(dc, sa);
    __m128i tmp = SkMin32_SSE2(tmp1, tmp2);

    __m128i ret1 = _mm_add_epi32(sc, dc);
    __m128i ret2 = _mm_slli_epi32(SkDiv255Round_SSE2(tmp), 1);
    __m128i ret = _mm_sub_epi32(ret1, ret2);

    ret = clamp_signed_byte_SSE2(ret);
    return ret;
}

static __m128i difference_modeproc_SSE2(const __m128i& src,
                                        const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = difference_byte_SSE2(SkGetPackedR32_SSE2(src),
                                     SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = difference_byte_SSE2(SkGetPackedG32_SSE2(src),
                                     SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = difference_byte_SSE2(SkGetPackedB32_SSE2(src),
                                     SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

static inline __m128i exclusion_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                          const __m128i&, __m128i&) {
    __m128i tmp1 = _mm_mullo_epi16(_mm_set1_epi32(255), sc); // 255 * sc
    __m128i tmp2 = _mm_mullo_epi16(_mm_set1_epi32(255), dc); // 255 * dc
    tmp1 = _mm_add_epi32(tmp1, tmp2);
    tmp2 = _mm_mullo_epi16(sc, dc);                          // sc * dc
    tmp2 = _mm_slli_epi32(tmp2, 1);                          // 2 * sc * dc

    __m128i r = _mm_sub_epi32(tmp1, tmp2);
    return clamp_div255round_SSE2(r);
}

static __m128i exclusion_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);

    __m128i a = srcover_byte_SSE2(sa, da);
    __m128i r = exclusion_byte_SSE2(SkGetPackedR32_SSE2(src),
                                    SkGetPackedR32_SSE2(dst), sa, da);
    __m128i g = exclusion_byte_SSE2(SkGetPackedG32_SSE2(src),
                                    SkGetPackedG32_SSE2(dst), sa, da);
    __m128i b = exclusion_byte_SSE2(SkGetPackedB32_SSE2(src),
                                    SkGetPackedB32_SSE2(dst), sa, da);
    return SkPackARGB32_SSE2(a, r, g, b);
}

////////////////////////////////////////////////////////////////////////////////

typedef __m128i (*SkXfermodeProcSIMD)(const __m128i& src, const __m128i& dst);

extern SkXfermodeProcSIMD gSSE2XfermodeProcs[];

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkSSE2ProcCoeffXfermode::SkSSE2ProcCoeffXfermode(SkReadBuffer& buffer) : INHERITED(buffer) {
    fProcSIMD = reinterpret_cast<void*>(gSSE2XfermodeProcs[this->getMode()]);
    buffer.validate(fProcSIMD != NULL);
}
#endif

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

////////////////////////////////////////////////////////////////////////////////

// 4 pixels modeprocs with SSE2
SkXfermodeProcSIMD gSSE2XfermodeProcs[] = {
    NULL, // kClear_Mode
    NULL, // kSrc_Mode
    NULL, // kDst_Mode
    srcover_modeproc_SSE2,
    dstover_modeproc_SSE2,
    srcin_modeproc_SSE2,
    dstin_modeproc_SSE2,
    srcout_modeproc_SSE2,
    dstout_modeproc_SSE2,
    srcatop_modeproc_SSE2,
    dstatop_modeproc_SSE2,
    xor_modeproc_SSE2,
    plus_modeproc_SSE2,
    modulate_modeproc_SSE2,
    screen_modeproc_SSE2,

    overlay_modeproc_SSE2,
    darken_modeproc_SSE2,
    lighten_modeproc_SSE2,
    colordodge_modeproc_SSE2,
    colorburn_modeproc_SSE2,
    hardlight_modeproc_SSE2,
    softlight_modeproc_SSE2,
    difference_modeproc_SSE2,
    exclusion_modeproc_SSE2,
    multiply_modeproc_SSE2,

    NULL, // kHue_Mode
    NULL, // kSaturation_Mode
    NULL, // kColor_Mode
    NULL, // kLuminosity_Mode
};

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_SSE2(const ProcCoeff& rec,
                                                         SkXfermode::Mode mode) {
    void* procSIMD = reinterpret_cast<void*>(gSSE2XfermodeProcs[mode]);

    if (procSIMD != NULL) {
        return SkNEW_ARGS(SkSSE2ProcCoeffXfermode, (rec, mode, procSIMD));
    }
    return NULL;
}
