#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"

///////////////////////////////////////////////////////////////////////////////

static void S32_D565_Opaque(uint16_t* SK_RESTRICT dst,
                            const SkPMColor* SK_RESTRICT src, int count,
                            U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);
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
            SkASSERT(SkGetPackedA32(c) == 255);
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
        int src_scale = SkAlpha255To256(alpha);
        do {
            SkPMColor sc = *src++;
            SkPMColorAssert(sc);
            if (sc)
            {
                uint16_t dc = *dst;
                unsigned sa = SkGetPackedA32(sc);
                unsigned dr, dg, db;
                
                if (sa == 255) {
                    dr = SkAlphaBlend(SkPacked32ToR16(sc), SkGetPackedR16(dc), src_scale);
                    dg = SkAlphaBlend(SkPacked32ToG16(sc), SkGetPackedG16(dc), src_scale);
                    db = SkAlphaBlend(SkPacked32ToB16(sc), SkGetPackedB16(dc), src_scale);
                } else {
                    unsigned dst_scale = 255 - SkAlphaMul(sa, src_scale);
                    dr = (SkPacked32ToR16(sc) * src_scale + SkGetPackedR16(dc) * dst_scale) >> 8;
                    dg = (SkPacked32ToG16(sc) * src_scale + SkGetPackedG16(dc) * dst_scale) >> 8;
                    db = (SkPacked32ToB16(sc) * src_scale + SkGetPackedB16(dc) * dst_scale) >> 8;
                }
                *dst = SkPackRGB16(dr, dg, db);
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
            SkASSERT(SkGetPackedA32(c) == 255);

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
            SkASSERT(SkGetPackedA32(c) == 255);

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

#ifdef USE_T32CB16BLEND_ASM
    extern "C" void scanline_t32cb16blend_arm(uint16_t*, uint32_t*, size_t);
#endif

static const SkBlitRow::Proc gProcs16[] = {
    // no dither
    S32_D565_Opaque,
    S32_D565_Blend,

#ifdef USE_T32CB16BLEND_ASM
    (SkBlitRow::Proc)scanline_t32cb16blend_arm,
#else
    S32A_D565_Opaque,
#endif

    S32A_D565_Blend,
    
    // dither
    S32_D565_Opaque_Dither,
    S32_D565_Blend_Dither,
    
    S32A_D565_Opaque_Dither,
    S32A_D565_Blend_Dither
};

extern SkBlitRow::Proc SkBlitRow_Factory_4444(unsigned flags);
    
SkBlitRow::Proc SkBlitRow::Factory(unsigned flags, SkBitmap::Config config) {
    SkASSERT(flags < SK_ARRAY_COUNT(gProcs16));
    
    switch (config) {
        case SkBitmap::kRGB_565_Config:
            return gProcs16[flags];
        case SkBitmap::kARGB_4444_Config:
            return SkBlitRow_Factory_4444(flags);
        default:
            break;
    }
    return NULL;
}
    
