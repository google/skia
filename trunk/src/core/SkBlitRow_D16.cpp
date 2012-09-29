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
                unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
                unsigned dr = SkMulS16(SkPacked32ToR16(sc), alpha) + SkMulS16(SkGetPackedR16(dc), dst_scale);
                unsigned dg = SkMulS16(SkPacked32ToG16(sc), alpha) + SkMulS16(SkGetPackedG16(dc), dst_scale);
                unsigned db = SkMulS16(SkPacked32ToB16(sc), alpha) + SkMulS16(SkGetPackedB16(dc), dst_scale);
                *dst = SkPackRGB16(SkDiv255Round(dr), SkDiv255Round(dg), SkDiv255Round(db));
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
                int dst_scale = SkAlpha255To256(255 - SkAlphaMul(sa, src_scale));
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
///////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc gDefault_565_Procs[] = {
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

extern SkBlitRow::Proc SkBlitRow_Factory_4444(unsigned flags);

SkBlitRow::Proc SkBlitRow::Factory(unsigned flags, SkBitmap::Config config) {
    SkASSERT(flags < SK_ARRAY_COUNT(gDefault_565_Procs));
    // just so we don't crash
    flags &= kFlags16_Mask;

    SkBlitRow::Proc proc = NULL;

    switch (config) {
        case SkBitmap::kRGB_565_Config:
            proc = PlatformProcs565(flags);
            if (NULL == proc) {
                proc = gDefault_565_Procs[flags];
            }
            break;
        case SkBitmap::kARGB_4444_Config:
            proc = PlatformProcs4444(flags);
            if (NULL == proc) {
                proc = SkBlitRow_Factory_4444(flags);
            }
            break;
        default:
            break;
    }
    return proc;
}

