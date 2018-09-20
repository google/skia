/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorData_DEFINED
#define SkColorData_DEFINED

// turn this own for extra debug checking when blending onto 565
#ifdef SK_DEBUG
    #define CHECK_FOR_565_OVERFLOW
#endif

#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkTo.h"

//////////////////////////////////////////////////////////////////////////////

#define SkASSERT_IS_BYTE(x)     SkASSERT(0 == ((x) & ~0xFF))

/*
 *  Skia's 32bit backend only supports 1 sizzle order at a time (compile-time).
 *  This is specified by 4 defines SK_A32_SHIFT, SK_R32_SHIFT, ... for G and B.
 *
 *  For easier compatibility with Skia's GPU backend, we further restrict these
 *  to either (in memory-byte-order) RGBA or BGRA. Note that this "order" does
 *  not directly correspond to the same shift-order, since we have to take endianess
 *  into account.
 *
 *  Here we enforce this constraint.
 */

#ifdef SK_CPU_BENDIAN
    #define SK_BGRA_B32_SHIFT   24
    #define SK_BGRA_G32_SHIFT   16
    #define SK_BGRA_R32_SHIFT   8
    #define SK_BGRA_A32_SHIFT   0
#else
    #define SK_BGRA_B32_SHIFT   0
    #define SK_BGRA_G32_SHIFT   8
    #define SK_BGRA_R32_SHIFT   16
    #define SK_BGRA_A32_SHIFT   24
#endif

#if defined(SK_PMCOLOR_IS_RGBA) && defined(SK_PMCOLOR_IS_BGRA)
    #error "can't define PMCOLOR to be RGBA and BGRA"
#endif

#define LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_RGBA  \
    (SK_A32_SHIFT == SK_RGBA_A32_SHIFT &&    \
     SK_R32_SHIFT == SK_RGBA_R32_SHIFT &&    \
     SK_G32_SHIFT == SK_RGBA_G32_SHIFT &&    \
     SK_B32_SHIFT == SK_RGBA_B32_SHIFT)

#define LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_BGRA  \
    (SK_A32_SHIFT == SK_BGRA_A32_SHIFT &&    \
     SK_R32_SHIFT == SK_BGRA_R32_SHIFT &&    \
     SK_G32_SHIFT == SK_BGRA_G32_SHIFT &&    \
     SK_B32_SHIFT == SK_BGRA_B32_SHIFT)


#define SK_A_INDEX  (SK_A32_SHIFT/8)
#define SK_R_INDEX  (SK_R32_SHIFT/8)
#define SK_G_INDEX  (SK_G32_SHIFT/8)
#define SK_B_INDEX  (SK_B32_SHIFT/8)

#if defined(SK_PMCOLOR_IS_RGBA) && !LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_RGBA
    #error "SK_PMCOLOR_IS_RGBA does not match SK_*32_SHIFT values"
#endif

#if defined(SK_PMCOLOR_IS_BGRA) && !LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_BGRA
    #error "SK_PMCOLOR_IS_BGRA does not match SK_*32_SHIFT values"
#endif

#if !defined(SK_PMCOLOR_IS_RGBA) && !defined(SK_PMCOLOR_IS_BGRA)
    // deduce which to define from the _SHIFT defines

    #if LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_RGBA
        #define SK_PMCOLOR_IS_RGBA
    #elif LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_BGRA
        #define SK_PMCOLOR_IS_BGRA
    #else
        #error "need 32bit packing to be either RGBA or BGRA"
    #endif
#endif

// hide these now that we're done
#undef LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_RGBA
#undef LOCAL_PMCOLOR_SHIFTS_EQUIVALENT_TO_BGRA

//////////////////////////////////////////////////////////////////////////////

// Reverse the bytes coorsponding to RED and BLUE in a packed pixels. Note the
// pair of them are in the same 2 slots in both RGBA and BGRA, thus there is
// no need to pass in the colortype to this function.
static inline uint32_t SkSwizzle_RB(uint32_t c) {
    static const uint32_t kRBMask = (0xFF << SK_R32_SHIFT) | (0xFF << SK_B32_SHIFT);

    unsigned c0 = (c >> SK_R32_SHIFT) & 0xFF;
    unsigned c1 = (c >> SK_B32_SHIFT) & 0xFF;
    return (c & ~kRBMask) | (c0 << SK_B32_SHIFT) | (c1 << SK_R32_SHIFT);
}

static inline uint32_t SkPackARGB_as_RGBA(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    SkASSERT_IS_BYTE(a);
    SkASSERT_IS_BYTE(r);
    SkASSERT_IS_BYTE(g);
    SkASSERT_IS_BYTE(b);
    return (a << SK_RGBA_A32_SHIFT) | (r << SK_RGBA_R32_SHIFT) |
           (g << SK_RGBA_G32_SHIFT) | (b << SK_RGBA_B32_SHIFT);
}

static inline uint32_t SkPackARGB_as_BGRA(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    SkASSERT_IS_BYTE(a);
    SkASSERT_IS_BYTE(r);
    SkASSERT_IS_BYTE(g);
    SkASSERT_IS_BYTE(b);
    return (a << SK_BGRA_A32_SHIFT) | (r << SK_BGRA_R32_SHIFT) |
           (g << SK_BGRA_G32_SHIFT) | (b << SK_BGRA_B32_SHIFT);
}

static inline SkPMColor SkSwizzle_RGBA_to_PMColor(uint32_t c) {
#ifdef SK_PMCOLOR_IS_RGBA
    return c;
#else
    return SkSwizzle_RB(c);
#endif
}

static inline SkPMColor SkSwizzle_BGRA_to_PMColor(uint32_t c) {
#ifdef SK_PMCOLOR_IS_BGRA
    return c;
#else
    return SkSwizzle_RB(c);
#endif
}

//////////////////////////////////////////////////////////////////////////////

///@{
/** See ITU-R Recommendation BT.709 at http://www.itu.int/rec/R-REC-BT.709/ .*/
#define SK_ITU_BT709_LUM_COEFF_R (0.2126f)
#define SK_ITU_BT709_LUM_COEFF_G (0.7152f)
#define SK_ITU_BT709_LUM_COEFF_B (0.0722f)
///@}

///@{
/** A float value which specifies this channel's contribution to luminance. */
#define SK_LUM_COEFF_R SK_ITU_BT709_LUM_COEFF_R
#define SK_LUM_COEFF_G SK_ITU_BT709_LUM_COEFF_G
#define SK_LUM_COEFF_B SK_ITU_BT709_LUM_COEFF_B
///@}

/** Computes the luminance from the given r, g, and b in accordance with
    SK_LUM_COEFF_X. For correct results, r, g, and b should be in linear space.
*/
static inline U8CPU SkComputeLuminance(U8CPU r, U8CPU g, U8CPU b) {
    //The following is
    //r * SK_LUM_COEFF_R + g * SK_LUM_COEFF_G + b * SK_LUM_COEFF_B
    //with SK_LUM_COEFF_X in 1.8 fixed point (rounding adjusted to sum to 256).
    return (r * 54 + g * 183 + b * 19) >> 8;
}

/**
 *  Turn a 0..255 value into a 0..256 value, rounding up if the value is >= 0x80.
 *  This is slightly more accurate than SkAlpha255To256.
 */
static inline unsigned Sk255To256(U8CPU value) {
    SkASSERT(SkToU8(value) == value);
    return value + (value >> 7);
}

/** Calculates 256 - (value * alpha256) / 255 in range [0,256],
 *  for [0,255] value and [0,256] alpha256.
 */
static inline U16CPU SkAlphaMulInv256(U16CPU value, U16CPU alpha256) {
    unsigned prod = 0xFFFF - value * alpha256;
    return (prod + (prod >> 8)) >> 8;
}

//  The caller may want negative values, so keep all params signed (int)
//  so we don't accidentally slip into unsigned math and lose the sign
//  extension when we shift (in SkAlphaMul)
static inline int SkAlphaBlend(int src, int dst, int scale256) {
    SkASSERT((unsigned)scale256 <= 256);
    return dst + SkAlphaMul(src - dst, scale256);
}

#define SkR16Assert(r)  SkASSERT((unsigned)(r) <= SK_R16_MASK)
#define SkG16Assert(g)  SkASSERT((unsigned)(g) <= SK_G16_MASK)
#define SkB16Assert(b)  SkASSERT((unsigned)(b) <= SK_B16_MASK)

static inline uint16_t SkPackRGB16(unsigned r, unsigned g, unsigned b) {
    SkASSERT(r <= SK_R16_MASK);
    SkASSERT(g <= SK_G16_MASK);
    SkASSERT(b <= SK_B16_MASK);

    return SkToU16((r << SK_R16_SHIFT) | (g << SK_G16_SHIFT) | (b << SK_B16_SHIFT));
}

#define SK_R16_MASK_IN_PLACE        (SK_R16_MASK << SK_R16_SHIFT)
#define SK_G16_MASK_IN_PLACE        (SK_G16_MASK << SK_G16_SHIFT)
#define SK_B16_MASK_IN_PLACE        (SK_B16_MASK << SK_B16_SHIFT)

/** Expand the 16bit color into a 32bit value that can be scaled all at once
    by a value up to 32. Used in conjunction with SkCompact_rgb_16.
*/
static inline uint32_t SkExpand_rgb_16(U16CPU c) {
    SkASSERT(c == (uint16_t)c);

    return ((c & SK_G16_MASK_IN_PLACE) << 16) | (c & ~SK_G16_MASK_IN_PLACE);
}

/** Compress an expanded value (from SkExpand_rgb_16) back down to a 16bit
    color value. The computation yields only 16bits of valid data, but we claim
    to return 32bits, so that the compiler won't generate extra instructions to
    "clean" the top 16bits. However, the top 16 can contain garbage, so it is
    up to the caller to safely ignore them.
*/
static inline U16CPU SkCompact_rgb_16(uint32_t c) {
    return ((c >> 16) & SK_G16_MASK_IN_PLACE) | (c & ~SK_G16_MASK_IN_PLACE);
}

/** Scale the 16bit color value by the 0..256 scale parameter.
    The computation yields only 16bits of valid data, but we claim
    to return 32bits, so that the compiler won't generate extra instructions to
    "clean" the top 16bits.
*/
static inline U16CPU SkAlphaMulRGB16(U16CPU c, unsigned scale) {
    return SkCompact_rgb_16(SkExpand_rgb_16(c) * (scale >> 3) >> 5);
}

// this helper explicitly returns a clean 16bit value (but slower)
#define SkAlphaMulRGB16_ToU16(c, s)  (uint16_t)SkAlphaMulRGB16(c, s)

/** Blend pre-expanded RGB32 with 16bit color value by the 0..32 scale parameter.
    The computation yields only 16bits of valid data, but we claim to return
    32bits, so that the compiler won't generate extra instructions to "clean"
    the top 16bits.
*/
static inline U16CPU SkBlend32_RGB16(uint32_t src_expand, uint16_t dst, unsigned scale) {
    uint32_t dst_expand = SkExpand_rgb_16(dst) * scale;
    return SkCompact_rgb_16((src_expand + dst_expand) >> 5);
}

/** Blend src and dst 16bit colors by the 0..256 scale parameter.
    The computation yields only 16bits of valid data, but we claim
    to return 32bits, so that the compiler won't generate extra instructions to
    "clean" the top 16bits.
*/
static inline U16CPU SkBlendRGB16(U16CPU src, U16CPU dst, int srcScale) {
    SkASSERT((unsigned)srcScale <= 256);

    srcScale >>= 3;

    uint32_t src32 = SkExpand_rgb_16(src);
    uint32_t dst32 = SkExpand_rgb_16(dst);
    return SkCompact_rgb_16(dst32 + ((src32 - dst32) * srcScale >> 5));
}

static inline void SkBlendRGB16(const uint16_t src[], uint16_t dst[],
                                int srcScale, int count) {
    SkASSERT(count > 0);
    SkASSERT((unsigned)srcScale <= 256);

    srcScale >>= 3;

    do {
        uint32_t src32 = SkExpand_rgb_16(*src++);
        uint32_t dst32 = SkExpand_rgb_16(*dst);
        *dst++ = static_cast<uint16_t>(
            SkCompact_rgb_16(dst32 + ((src32 - dst32) * srcScale >> 5)));
    } while (--count > 0);
}

#ifdef SK_DEBUG
    static inline U16CPU SkRGB16Add(U16CPU a, U16CPU b) {
        SkASSERT(SkGetPackedR16(a) + SkGetPackedR16(b) <= SK_R16_MASK);
        SkASSERT(SkGetPackedG16(a) + SkGetPackedG16(b) <= SK_G16_MASK);
        SkASSERT(SkGetPackedB16(a) + SkGetPackedB16(b) <= SK_B16_MASK);

        return a + b;
    }
#else
    #define SkRGB16Add(a, b)  ((a) + (b))
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    #define SkPMColorAssert(color_value)                                    \
        do {                                                                \
            SkPMColor pm_color_value = (color_value);                       \
            uint32_t alpha_color_value = SkGetPackedA32(pm_color_value);    \
            SkA32Assert(alpha_color_value);                                 \
            SkASSERT(SkGetPackedR32(pm_color_value) <= alpha_color_value);  \
            SkASSERT(SkGetPackedG32(pm_color_value) <= alpha_color_value);  \
            SkASSERT(SkGetPackedB32(pm_color_value) <= alpha_color_value);  \
        } while (false)
#else
    #define SkPMColorAssert(c)
#endif

static inline bool SkPMColorValid(SkPMColor c) {
    auto a = SkGetPackedA32(c);
    bool valid = a <= SK_A32_MASK
              && SkGetPackedR32(c) <= a
              && SkGetPackedG32(c) <= a
              && SkGetPackedB32(c) <= a;
    if (valid) {
        SkPMColorAssert(c);  // Make sure we're consistent when it counts.
    }
    return valid;
}

static inline uint32_t SkPackPMColor_as_RGBA(SkPMColor c) {
    return SkPackARGB_as_RGBA(SkGetPackedA32(c), SkGetPackedR32(c),
                              SkGetPackedG32(c), SkGetPackedB32(c));
}

static inline uint32_t SkPackPMColor_as_BGRA(SkPMColor c) {
    return SkPackARGB_as_BGRA(SkGetPackedA32(c), SkGetPackedR32(c),
                              SkGetPackedG32(c), SkGetPackedB32(c));
}

/**
 * Abstract 4-byte interpolation, implemented on top of SkPMColor
 * utility functions. Third parameter controls blending of the first two:
 *   (src, dst, 0) returns dst
 *   (src, dst, 0xFF) returns src
 *   srcWeight is [0..256], unlike SkFourByteInterp which takes [0..255]
 */
static inline SkPMColor SkFourByteInterp256(SkPMColor src, SkPMColor dst,
                                         unsigned scale) {
    unsigned a = SkAlphaBlend(SkGetPackedA32(src), SkGetPackedA32(dst), scale);
    unsigned r = SkAlphaBlend(SkGetPackedR32(src), SkGetPackedR32(dst), scale);
    unsigned g = SkAlphaBlend(SkGetPackedG32(src), SkGetPackedG32(dst), scale);
    unsigned b = SkAlphaBlend(SkGetPackedB32(src), SkGetPackedB32(dst), scale);

    return SkPackARGB32(a, r, g, b);
}

/**
 * Abstract 4-byte interpolation, implemented on top of SkPMColor
 * utility functions. Third parameter controls blending of the first two:
 *   (src, dst, 0) returns dst
 *   (src, dst, 0xFF) returns src
 */
static inline SkPMColor SkFourByteInterp(SkPMColor src, SkPMColor dst,
                                         U8CPU srcWeight) {
    unsigned scale = SkAlpha255To256(srcWeight);
    return SkFourByteInterp256(src, dst, scale);
}

/**
 * 0xAARRGGBB -> 0x00AA00GG, 0x00RR00BB
 */
static inline void SkSplay(uint32_t color, uint32_t* ag, uint32_t* rb) {
    const uint32_t mask = 0x00FF00FF;
    *ag = (color >> 8) & mask;
    *rb = color & mask;
}

/**
 * 0xAARRGGBB -> 0x00AA00GG00RR00BB
 * (note, ARGB -> AGRB)
 */
static inline uint64_t SkSplay(uint32_t color) {
    const uint32_t mask = 0x00FF00FF;
    uint64_t agrb = (color >> 8) & mask;  // 0x0000000000AA00GG
    agrb <<= 32;                          // 0x00AA00GG00000000
    agrb |= color & mask;                 // 0x00AA00GG00RR00BB
    return agrb;
}

/**
 * 0xAAxxGGxx, 0xRRxxBBxx-> 0xAARRGGBB
 */
static inline uint32_t SkUnsplay(uint32_t ag, uint32_t rb) {
    const uint32_t mask = 0xFF00FF00;
    return (ag & mask) | ((rb & mask) >> 8);
}

/**
 * 0xAAxxGGxxRRxxBBxx -> 0xAARRGGBB
 * (note, AGRB -> ARGB)
 */
static inline uint32_t SkUnsplay(uint64_t agrb) {
    const uint32_t mask = 0xFF00FF00;
    return SkPMColor(
        ((agrb & mask) >> 8) |   // 0x00RR00BB
        ((agrb >> 32) & mask));  // 0xAARRGGBB
}

static inline SkPMColor SkFastFourByteInterp256_32(SkPMColor src, SkPMColor dst, unsigned scale) {
    SkASSERT(scale <= 256);

    // Two 8-bit blends per two 32-bit registers, with space to make sure the math doesn't collide.
    uint32_t src_ag, src_rb, dst_ag, dst_rb;
    SkSplay(src, &src_ag, &src_rb);
    SkSplay(dst, &dst_ag, &dst_rb);

    const uint32_t ret_ag = src_ag * scale + (256 - scale) * dst_ag;
    const uint32_t ret_rb = src_rb * scale + (256 - scale) * dst_rb;

    return SkUnsplay(ret_ag, ret_rb);
}

static inline SkPMColor SkFastFourByteInterp256_64(SkPMColor src, SkPMColor dst, unsigned scale) {
    SkASSERT(scale <= 256);
    // Four 8-bit blends in one 64-bit register, with space to make sure the math doesn't collide.
    return SkUnsplay(SkSplay(src) * scale + (256-scale) * SkSplay(dst));
}

// TODO(mtklein): Replace slow versions with fast versions, using scale + (scale>>7) everywhere.

/**
 * Same as SkFourByteInterp256, but faster.
 */
static inline SkPMColor SkFastFourByteInterp256(SkPMColor src, SkPMColor dst, unsigned scale) {
    // On a 64-bit machine, _64 is about 10% faster than _32, but ~40% slower on a 32-bit machine.
    if (sizeof(void*) == 4) {
        return SkFastFourByteInterp256_32(src, dst, scale);
    } else {
        return SkFastFourByteInterp256_64(src, dst, scale);
    }
}

/**
 * Nearly the same as SkFourByteInterp, but faster and a touch more accurate, due to better
 * srcWeight scaling to [0, 256].
 */
static inline SkPMColor SkFastFourByteInterp(SkPMColor src,
                                             SkPMColor dst,
                                             U8CPU srcWeight) {
    SkASSERT(srcWeight <= 255);
    // scale = srcWeight + (srcWeight >> 7) is more accurate than
    // scale = srcWeight + 1, but 7% slower
    return SkFastFourByteInterp256(src, dst, srcWeight + (srcWeight >> 7));
}

/**
 * Interpolates between colors src and dst using [0,256] scale.
 */
static inline SkPMColor SkPMLerp(SkPMColor src, SkPMColor dst, unsigned scale) {
    return SkFastFourByteInterp256(src, dst, scale);
}

static inline SkPMColor SkBlendARGB32(SkPMColor src, SkPMColor dst, U8CPU aa) {
    SkASSERT((unsigned)aa <= 255);

    unsigned src_scale = SkAlpha255To256(aa);
    unsigned dst_scale = SkAlphaMulInv256(SkGetPackedA32(src), src_scale);

    const uint32_t mask = 0xFF00FF;

    uint32_t src_rb = (src & mask) * src_scale;
    uint32_t src_ag = ((src >> 8) & mask) * src_scale;

    uint32_t dst_rb = (dst & mask) * dst_scale;
    uint32_t dst_ag = ((dst >> 8) & mask) * dst_scale;

    return (((src_rb + dst_rb) >> 8) & mask) | ((src_ag + dst_ag) & ~mask);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Convert a 32bit pixel to a 16bit pixel (no dither)

#define SkR32ToR16_MACRO(r)   ((unsigned)(r) >> (SK_R32_BITS - SK_R16_BITS))
#define SkG32ToG16_MACRO(g)   ((unsigned)(g) >> (SK_G32_BITS - SK_G16_BITS))
#define SkB32ToB16_MACRO(b)   ((unsigned)(b) >> (SK_B32_BITS - SK_B16_BITS))

#ifdef SK_DEBUG
    static inline unsigned SkR32ToR16(unsigned r) {
        SkR32Assert(r);
        return SkR32ToR16_MACRO(r);
    }
    static inline unsigned SkG32ToG16(unsigned g) {
        SkG32Assert(g);
        return SkG32ToG16_MACRO(g);
    }
    static inline unsigned SkB32ToB16(unsigned b) {
        SkB32Assert(b);
        return SkB32ToB16_MACRO(b);
    }
#else
    #define SkR32ToR16(r)   SkR32ToR16_MACRO(r)
    #define SkG32ToG16(g)   SkG32ToG16_MACRO(g)
    #define SkB32ToB16(b)   SkB32ToB16_MACRO(b)
#endif

static inline U16CPU SkPixel32ToPixel16(SkPMColor c) {
    unsigned r = ((c >> (SK_R32_SHIFT + (8 - SK_R16_BITS))) & SK_R16_MASK) << SK_R16_SHIFT;
    unsigned g = ((c >> (SK_G32_SHIFT + (8 - SK_G16_BITS))) & SK_G16_MASK) << SK_G16_SHIFT;
    unsigned b = ((c >> (SK_B32_SHIFT + (8 - SK_B16_BITS))) & SK_B16_MASK) << SK_B16_SHIFT;
    return r | g | b;
}

static inline U16CPU SkPack888ToRGB16(U8CPU r, U8CPU g, U8CPU b) {
    return  (SkR32ToR16(r) << SK_R16_SHIFT) |
            (SkG32ToG16(g) << SK_G16_SHIFT) |
            (SkB32ToB16(b) << SK_B16_SHIFT);
}

#define SkPixel32ToPixel16_ToU16(src)   SkToU16(SkPixel32ToPixel16(src))

/////////////////////////////////////////////////////////////////////////////////////////

/*  SrcOver the 32bit src color with the 16bit dst, returning a 16bit value
    (with dirt in the high 16bits, so caller beware).
*/
static inline U16CPU SkSrcOver32To16(SkPMColor src, uint16_t dst) {
    unsigned sr = SkGetPackedR32(src);
    unsigned sg = SkGetPackedG32(src);
    unsigned sb = SkGetPackedB32(src);

    unsigned dr = SkGetPackedR16(dst);
    unsigned dg = SkGetPackedG16(dst);
    unsigned db = SkGetPackedB16(dst);

    unsigned isa = 255 - SkGetPackedA32(src);

    dr = (sr + SkMul16ShiftRound(dr, isa, SK_R16_BITS)) >> (8 - SK_R16_BITS);
    dg = (sg + SkMul16ShiftRound(dg, isa, SK_G16_BITS)) >> (8 - SK_G16_BITS);
    db = (sb + SkMul16ShiftRound(db, isa, SK_B16_BITS)) >> (8 - SK_B16_BITS);

    return SkPackRGB16(dr, dg, db);
}

static inline SkColor SkPixel16ToColor(U16CPU src) {
    SkASSERT(src == SkToU16(src));

    unsigned    r = SkPacked16ToR32(src);
    unsigned    g = SkPacked16ToG32(src);
    unsigned    b = SkPacked16ToB32(src);

    SkASSERT((r >> (8 - SK_R16_BITS)) == SkGetPackedR16(src));
    SkASSERT((g >> (8 - SK_G16_BITS)) == SkGetPackedG16(src));
    SkASSERT((b >> (8 - SK_B16_BITS)) == SkGetPackedB16(src));

    return SkColorSetRGB(r, g, b);
}

///////////////////////////////////////////////////////////////////////////////

typedef uint16_t SkPMColor16;

// Put in OpenGL order (r g b a)
#define SK_A4444_SHIFT    0
#define SK_R4444_SHIFT    12
#define SK_G4444_SHIFT    8
#define SK_B4444_SHIFT    4

#define SkA32To4444(a)  ((unsigned)(a) >> 4)
#define SkR32To4444(r)  ((unsigned)(r) >> 4)
#define SkG32To4444(g)  ((unsigned)(g) >> 4)
#define SkB32To4444(b)  ((unsigned)(b) >> 4)

static inline U8CPU SkReplicateNibble(unsigned nib) {
    SkASSERT(nib <= 0xF);
    return (nib << 4) | nib;
}

#define SkGetPackedA4444(c)     (((unsigned)(c) >> SK_A4444_SHIFT) & 0xF)
#define SkGetPackedR4444(c)     (((unsigned)(c) >> SK_R4444_SHIFT) & 0xF)
#define SkGetPackedG4444(c)     (((unsigned)(c) >> SK_G4444_SHIFT) & 0xF)
#define SkGetPackedB4444(c)     (((unsigned)(c) >> SK_B4444_SHIFT) & 0xF)

#define SkPacked4444ToA32(c)    SkReplicateNibble(SkGetPackedA4444(c))
#define SkPacked4444ToR32(c)    SkReplicateNibble(SkGetPackedR4444(c))
#define SkPacked4444ToG32(c)    SkReplicateNibble(SkGetPackedG4444(c))
#define SkPacked4444ToB32(c)    SkReplicateNibble(SkGetPackedB4444(c))

static inline SkPMColor16 SkPackARGB4444(unsigned a, unsigned r,
                                         unsigned g, unsigned b) {
    SkASSERT(a <= 0xF);
    SkASSERT(r <= a);
    SkASSERT(g <= a);
    SkASSERT(b <= a);

    return (SkPMColor16)((a << SK_A4444_SHIFT) | (r << SK_R4444_SHIFT) |
                         (g << SK_G4444_SHIFT) | (b << SK_B4444_SHIFT));
}

/** Expand the SkPMColor16 color into a 32bit value that can be scaled all at
    once by a value up to 16.
*/
static inline uint32_t SkExpand_4444(U16CPU c) {
    SkASSERT(c == (uint16_t)c);

    const unsigned mask = 0xF0F;    //gMask_0F0F;
    return (c & mask) | ((c & ~mask) << 12);
}

static inline SkPMColor SkPixel4444ToPixel32(U16CPU c) {
    uint32_t d = (SkGetPackedA4444(c) << SK_A32_SHIFT) |
                 (SkGetPackedR4444(c) << SK_R32_SHIFT) |
                 (SkGetPackedG4444(c) << SK_G32_SHIFT) |
                 (SkGetPackedB4444(c) << SK_B32_SHIFT);
    return d | (d << 4);
}

///////////////////////////////////////////////////////////////////////////////

static inline int SkUpscale31To32(int value) {
    SkASSERT((unsigned)value <= 31);
    return value + (value >> 4);
}

static inline int SkBlend32(int src, int dst, int scale) {
    SkASSERT((unsigned)src <= 0xFF);
    SkASSERT((unsigned)dst <= 0xFF);
    SkASSERT((unsigned)scale <= 32);
    return dst + ((src - dst) * scale >> 5);
}

static inline SkPMColor SkBlendLCD16(int srcA, int srcR, int srcG, int srcB,
                                     SkPMColor dst, uint16_t mask) {
    if (mask == 0) {
        return dst;
    }

    /*  We want all of these in 5bits, hence the shifts in case one of them
     *  (green) is 6bits.
     */
    int maskR = SkGetPackedR16(mask) >> (SK_R16_BITS - 5);
    int maskG = SkGetPackedG16(mask) >> (SK_G16_BITS - 5);
    int maskB = SkGetPackedB16(mask) >> (SK_B16_BITS - 5);

    // Now upscale them to 0..32, so we can use blend32
    maskR = SkUpscale31To32(maskR);
    maskG = SkUpscale31To32(maskG);
    maskB = SkUpscale31To32(maskB);

    // srcA has been upscaled to 256 before passed into this function
    maskR = maskR * srcA >> 8;
    maskG = maskG * srcA >> 8;
    maskB = maskB * srcA >> 8;

    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    // LCD blitting is only supported if the dst is known/required
    // to be opaque
    return SkPackARGB32(0xFF,
                        SkBlend32(srcR, dstR, maskR),
                        SkBlend32(srcG, dstG, maskG),
                        SkBlend32(srcB, dstB, maskB));
}

static inline SkPMColor SkBlendLCD16Opaque(int srcR, int srcG, int srcB,
                                           SkPMColor dst, uint16_t mask,
                                           SkPMColor opaqueDst) {
    if (mask == 0) {
        return dst;
    }

    if (0xFFFF == mask) {
        return opaqueDst;
    }

    /*  We want all of these in 5bits, hence the shifts in case one of them
     *  (green) is 6bits.
     */
    int maskR = SkGetPackedR16(mask) >> (SK_R16_BITS - 5);
    int maskG = SkGetPackedG16(mask) >> (SK_G16_BITS - 5);
    int maskB = SkGetPackedB16(mask) >> (SK_B16_BITS - 5);

    // Now upscale them to 0..32, so we can use blend32
    maskR = SkUpscale31To32(maskR);
    maskG = SkUpscale31To32(maskG);
    maskB = SkUpscale31To32(maskB);

    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    // LCD blitting is only supported if the dst is known/required
    // to be opaque
    return SkPackARGB32(0xFF,
                        SkBlend32(srcR, dstR, maskR),
                        SkBlend32(srcG, dstG, maskG),
                        SkBlend32(srcB, dstB, maskB));
}

static inline void SkBlitLCD16Row(SkPMColor dst[], const uint16_t mask[],
                                  SkColor src, int width, SkPMColor) {
    int srcA = SkColorGetA(src);
    int srcR = SkColorGetR(src);
    int srcG = SkColorGetG(src);
    int srcB = SkColorGetB(src);

    srcA = SkAlpha255To256(srcA);

    for (int i = 0; i < width; i++) {
        dst[i] = SkBlendLCD16(srcA, srcR, srcG, srcB, dst[i], mask[i]);
    }
}

static inline void SkBlitLCD16OpaqueRow(SkPMColor dst[], const uint16_t mask[],
                                        SkColor src, int width,
                                        SkPMColor opaqueDst) {
    int srcR = SkColorGetR(src);
    int srcG = SkColorGetG(src);
    int srcB = SkColorGetB(src);

    for (int i = 0; i < width; i++) {
        dst[i] = SkBlendLCD16Opaque(srcR, srcG, srcB, dst[i], mask[i],
                                    opaqueDst);
    }
}

#endif
