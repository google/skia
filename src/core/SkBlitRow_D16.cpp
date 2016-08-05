/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"
#include "SkMathPriv.h"

///////////////////////////////////////////////////////////////////////////////

static void S32_D565_Opaque(uint16_t* SK_RESTRICT dst,
                            const SkPMColor* SK_RESTRICT src, int count,
                            U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            *dst++ = SkPixel32ToPixel16_ToU16(c);
        } while (--count != 0);
    }
}

static void S32_D565_Blend(uint16_t* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 > alpha);

    if (count > 0) {
        int scale = SkAlpha255To256(alpha);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            uint16_t d = *dst;
            *dst++ = SkPackRGB16(
                    SkAlphaBlend(SkPacked32ToR16(c), SkGetPackedR16(d), scale),
                    SkAlphaBlend(SkPacked32ToG16(c), SkGetPackedG16(d), scale),
                    SkAlphaBlend(SkPacked32ToB16(c), SkGetPackedB16(d), scale));
        } while (--count != 0);
    }
}

static void S32A_D565_Opaque(uint16_t* SK_RESTRICT dst,
                               const SkPMColor* SK_RESTRICT src, int count,
                               U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
//            if (__builtin_expect(c!=0, 1))
            if (c) {
                *dst = SkSrcOver32To16(c, *dst);
            }
            dst += 1;
        } while (--count != 0);
    }
}

static void S32A_D565_Blend(uint16_t* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                               U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 > alpha);

    if (count > 0) {
        do {
            SkPMColor sc = *src++;
            SkPMColorAssert(sc);
            if (sc) {
                uint16_t dc = *dst;
                SkPMColor res = SkBlendARGB32(sc, SkPixel16ToPixel32(dc), alpha);
                *dst = SkPixel32ToPixel16(res);
            }
            dst += 1;
        } while (--count != 0);
    }
}

/////////////////////////////////////////////////////////////////////////////

static void S32_D565_Opaque_Dither(uint16_t* SK_RESTRICT dst,
                                     const SkPMColor* SK_RESTRICT src,
                                     int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            unsigned dither = DITHER_VALUE(x);
            *dst++ = SkDitherRGB32To565(c, dither);
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

static void S32_D565_Blend_Dither(uint16_t* SK_RESTRICT dst,
                                    const SkPMColor* SK_RESTRICT src,
                                    int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 > alpha);

    if (count > 0) {
        int scale = SkAlpha255To256(alpha);
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            int dither = DITHER_VALUE(x);
            int sr = SkGetPackedR32(c);
            int sg = SkGetPackedG32(c);
            int sb = SkGetPackedB32(c);
            sr = SkDITHER_R32To565(sr, dither);
            sg = SkDITHER_G32To565(sg, dither);
            sb = SkDITHER_B32To565(sb, dither);

            uint16_t d = *dst;
            *dst++ = SkPackRGB16(SkAlphaBlend(sr, SkGetPackedR16(d), scale),
                                 SkAlphaBlend(sg, SkGetPackedG16(d), scale),
                                 SkAlphaBlend(sb, SkGetPackedB16(d), scale));
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

static void S32A_D565_Opaque_Dither(uint16_t* SK_RESTRICT dst,
                                      const SkPMColor* SK_RESTRICT src,
                                      int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                unsigned a = SkGetPackedA32(c);

                int d = SkAlphaMul(DITHER_VALUE(x), SkAlpha255To256(a));

                unsigned sr = SkGetPackedR32(c);
                unsigned sg = SkGetPackedG32(c);
                unsigned sb = SkGetPackedB32(c);
                sr = SkDITHER_R32_FOR_565(sr, d);
                sg = SkDITHER_G32_FOR_565(sg, d);
                sb = SkDITHER_B32_FOR_565(sb, d);

                uint32_t src_expanded = (sg << 24) | (sr << 13) | (sb << 2);
                uint32_t dst_expanded = SkExpand_rgb_16(*dst);
                dst_expanded = dst_expanded * (SkAlpha255To256(255 - a) >> 3);
                // now src and dst expanded are in g:11 r:10 x:1 b:10
                *dst = SkCompact_rgb_16((src_expanded + dst_expanded) >> 5);
            }
            dst += 1;
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

static void S32A_D565_Blend_Dither(uint16_t* SK_RESTRICT dst,
                                     const SkPMColor* SK_RESTRICT src,
                                     int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 > alpha);

    if (count > 0) {
        int src_scale = SkAlpha255To256(alpha);
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c)
            {
                unsigned d = *dst;
                int sa = SkGetPackedA32(c);
                int dst_scale = SkAlphaMulInv256(sa, src_scale);
                int dither = DITHER_VALUE(x);

                int sr = SkGetPackedR32(c);
                int sg = SkGetPackedG32(c);
                int sb = SkGetPackedB32(c);
                sr = SkDITHER_R32To565(sr, dither);
                sg = SkDITHER_G32To565(sg, dither);
                sb = SkDITHER_B32To565(sb, dither);

                int dr = (sr * src_scale + SkGetPackedR16(d) * dst_scale) >> 8;
                int dg = (sg * src_scale + SkGetPackedG16(d) * dst_scale) >> 8;
                int db = (sb * src_scale + SkGetPackedB16(d) * dst_scale) >> 8;

                *dst = SkPackRGB16(dr, dg, db);
            }
            dst += 1;
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

static uint32_t pmcolor_to_expand16(SkPMColor c) {
    unsigned r = SkGetPackedR32(c);
    unsigned g = SkGetPackedG32(c);
    unsigned b = SkGetPackedB32(c);
    return (g << 24) | (r << 13) | (b << 2);
}

static void Color32A_D565(uint16_t dst[], SkPMColor src, int count, int x, int y) {
    SkASSERT(count > 0);
    uint32_t src_expand = pmcolor_to_expand16(src);
    unsigned scale = SkAlpha255To256(0xFF - SkGetPackedA32(src)) >> 3;
    do {
        *dst = SkBlend32_RGB16(src_expand, *dst, scale);
        dst += 1;
    } while (--count != 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc16 gDefault_565_Procs[] = {
    // no dither
    S32_D565_Opaque,
    S32_D565_Blend,

    S32A_D565_Opaque,
    S32A_D565_Blend,

    // dither
    S32_D565_Opaque_Dither,
    S32_D565_Blend_Dither,

    S32A_D565_Opaque_Dither,
    S32A_D565_Blend_Dither
};

SkBlitRow::Proc16 SkBlitRow::Factory16(unsigned flags) {
    SkASSERT(flags < SK_ARRAY_COUNT(gDefault_565_Procs));
    // just so we don't crash
    flags &= kFlags16_Mask;

    SkBlitRow::Proc16 proc = PlatformFactory565(flags);
    if (nullptr == proc) {
        proc = gDefault_565_Procs[flags];
    }
    return proc;
}

static const SkBlitRow::ColorProc16 gDefault_565_ColorProcs[] = {
#if 0
    Color32A_D565,
    Color32A_D565_Dither
#else
    // TODO: stop cheating and fill dither from the above specializations!
    Color32A_D565,
    Color32A_D565,
#endif
};

SkBlitRow::ColorProc16 SkBlitRow::ColorFactory16(unsigned flags) {
    SkASSERT((flags & ~kFlags16_Mask) == 0);
    // just so we don't crash
    flags &= kFlags16_Mask;
    // we ignore both kGlobalAlpha_Flag and kSrcPixelAlpha_Flag, so shift down
    // no need for the additional code specializing on opaque alpha at this time
    flags >>= 2;

    SkASSERT(flags < SK_ARRAY_COUNT(gDefault_565_ColorProcs));

    SkBlitRow::ColorProc16 proc = PlatformColorFactory565(flags);
    if (nullptr == proc) {
        proc = gDefault_565_ColorProcs[flags];
    }
    return proc;
}
