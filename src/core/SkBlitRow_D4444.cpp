/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"

///////////////////////////////////////////////////////////////////////////////

static void S32_D4444_Opaque(uint16_t* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);
            *dst++ = SkPixel32ToPixel4444(c);
        } while (--count != 0);
    }
}

static void S32_D4444_Blend(uint16_t* SK_RESTRICT dst,
                            const SkPMColor* SK_RESTRICT src, int count,
                            U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 > alpha);

    if (count > 0) {
        unsigned scale16 = SkAlpha255To256(alpha) >> 4;
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);

            uint32_t src_expand = SkExpand32_4444(c);
            uint32_t dst_expand = SkExpand_4444(*dst);
            dst_expand += (src_expand - dst_expand) * scale16 >> 4;
            *dst++ = SkCompact_4444(dst_expand);
        } while (--count != 0);
    }
}

static void S32A_D4444_Opaque(uint16_t* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                              U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
//            if (__builtin_expect(c!=0, 1))
            if (c)
            {
                unsigned scale16 = SkAlpha255To256(255 - SkGetPackedA32(c)) >> 4;
                uint32_t src_expand = SkExpand_8888(c);
                uint32_t dst_expand = SkExpand_4444(*dst) * scale16;
                *dst = SkCompact_4444((src_expand + dst_expand) >> 4);
            }
            dst += 1;
        } while (--count != 0);
    }
}

static void S32A_D4444_Blend(uint16_t* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 > alpha);
    
    if (count > 0) {
        int src_scale = SkAlpha255To256(alpha) >> 4;
        do {
            SkPMColor sc = *src++;
            SkPMColorAssert(sc);

            if (sc) {
                unsigned dst_scale = 16 - (SkGetPackedA32(sc) * src_scale >> 8);
                uint32_t src_expand = SkExpand32_4444(sc) * src_scale;
                uint32_t dst_expand = SkExpand_4444(*dst) * dst_scale;
                *dst = SkCompact_4444((src_expand + dst_expand) >> 4);
            }
            dst += 1;
        } while (--count != 0);
    }
}

/////////////////////////////////////////////////////////////////////////////
                               
static void S32_D4444_Opaque_Dither(uint16_t* SK_RESTRICT dst,
                                    const SkPMColor* SK_RESTRICT src,
                                    int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);
    
    if (count > 0) {
        DITHER_4444_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            unsigned dither = DITHER_VALUE(x);
            *dst++ = SkDitherARGB32To4444(c, dither);
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

static void S32_D4444_Blend_Dither(uint16_t* SK_RESTRICT dst,
                                   const SkPMColor* SK_RESTRICT src,
                                   int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 > alpha);
    
    if (count > 0) {
        int scale16 = SkAlpha255To256(alpha) >> 4;
        DITHER_4444_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);

            uint32_t src_expand = SkExpand32_4444(c) * scale16;
            uint32_t dst_expand = SkExpand_4444(*dst) * (16 - scale16);
            
            c = SkCompact_8888(src_expand + dst_expand); // convert back to SkPMColor
            *dst++ = SkDitherARGB32To4444(c, DITHER_VALUE(x));
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

static void S32A_D4444_Opaque_Dither(uint16_t* SK_RESTRICT dst,
                                     const SkPMColor* SK_RESTRICT src,
                                     int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);
    
    if (count > 0) {
        DITHER_4444_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                unsigned a = SkGetPackedA32(c);                
                int d = SkAlphaMul(DITHER_VALUE(x), SkAlpha255To256(a));
                
                unsigned scale16 = SkAlpha255To256(255 - a) >> 4;
                uint32_t src_expand = SkExpand_8888(c);
                uint32_t dst_expand = SkExpand_4444(*dst) * scale16;
                // convert back to SkPMColor
                c = SkCompact_8888(src_expand + dst_expand);
                *dst = SkDitherARGB32To4444(c, d);
            }
            dst += 1;
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

// need DitherExpand888To4444(expand, dither)

static void S32A_D4444_Blend_Dither(uint16_t* SK_RESTRICT dst,
                                    const SkPMColor* SK_RESTRICT src,
                                    int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 > alpha);
    
    if (count > 0) {
        int src_scale = SkAlpha255To256(alpha) >> 4;
        DITHER_4444_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                unsigned a = SkAlpha255To256(SkGetPackedA32(c));
                int d = SkAlphaMul(DITHER_VALUE(x), a);
                
                unsigned dst_scale = 16 - SkAlphaMul(src_scale, a);
                uint32_t src_expand = SkExpand32_4444(c) * src_scale;
                uint32_t dst_expand = SkExpand_4444(*dst) * dst_scale;
                // convert back to SkPMColor
                c = SkCompact_8888(src_expand + dst_expand);
                *dst = SkDitherARGB32To4444(c, d);
            }
            dst += 1;
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc gProcs4444[] = {
    // no dither
    S32_D4444_Opaque,
    S32_D4444_Blend,
    
    S32A_D4444_Opaque,
    S32A_D4444_Blend,
    
    // dither
    S32_D4444_Opaque_Dither,
    S32_D4444_Blend_Dither,
    
    S32A_D4444_Opaque_Dither,
    S32A_D4444_Blend_Dither
};
    
SkBlitRow::Proc SkBlitRow_Factory_4444(unsigned flags);
SkBlitRow::Proc SkBlitRow_Factory_4444(unsigned flags)
{
    SkASSERT(flags < SK_ARRAY_COUNT(gProcs4444));
    
    return gProcs4444[flags];
}
    
    
