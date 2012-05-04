/* NEON optimized code (C) COPYRIGHT 2009 Motorola 
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkPerspIter.h"
#include "SkShader.h"
#include "SkUtils.h"

// Helper to ensure that when we shift down, we do it w/o sign-extension
// so the caller doesn't have to manually mask off the top 16 bits
//
static unsigned SK_USHIFT16(unsigned x) {
    return x >> 16;
}

/*  returns 0...(n-1) given any x (positive or negative).
    
    As an example, if n (which is always positive) is 5...
 
          x: -8 -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7  8
    returns:  2  3  4  0  1  2  3  4  0  1  2  3  4  0  1  2  3
 */
static inline int sk_int_mod(int x, int n) {
    SkASSERT(n > 0);
    if ((unsigned)x >= (unsigned)n) {
        if (x < 0) {
            x = n + ~(~x % n);
        } else {
            x = x % n;
        }
    }
    return x;
}

void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count);
void decal_filter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count);

#define MAKENAME(suffix)        ClampX_ClampY ## suffix
#define TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)    SkClampMax((fy) >> 16, max)
#define TILEX_LOW_BITS(fx, max) (((fx) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) (((fy) >> 12) & 0xF)
#define CHECK_FOR_DECAL
#if	defined(__ARM_HAVE_NEON)
    #include "SkBitmapProcState_matrix_clamp.h"
#else
    #include "SkBitmapProcState_matrix.h"
#endif

#define MAKENAME(suffix)        RepeatX_RepeatY ## suffix
#define TILEX_PROCF(fx, max)    SK_USHIFT16(((fx) & 0xFFFF) * ((max) + 1))
#define TILEY_PROCF(fy, max)    SK_USHIFT16(((fy) & 0xFFFF) * ((max) + 1))
#define TILEX_LOW_BITS(fx, max) ((((fx) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#define TILEY_LOW_BITS(fy, max) ((((fy) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#if	defined(__ARM_HAVE_NEON)
    #include "SkBitmapProcState_matrix_repeat.h"
#else
    #include "SkBitmapProcState_matrix.h"
#endif

#define MAKENAME(suffix)        GeneralXY ## suffix
#define PREAMBLE(state)         SkBitmapProcState::FixedTileProc tileProcX = (state).fTileProcX; \
                                SkBitmapProcState::FixedTileProc tileProcY = (state).fTileProcY; \
                                SkBitmapProcState::FixedTileLowBitsProc tileLowBitsProcX = (state).fTileLowBitsProcX; \
                                SkBitmapProcState::FixedTileLowBitsProc tileLowBitsProcY = (state).fTileLowBitsProcY
#define PREAMBLE_PARAM_X        , SkBitmapProcState::FixedTileProc tileProcX, SkBitmapProcState::FixedTileLowBitsProc tileLowBitsProcX
#define PREAMBLE_PARAM_Y        , SkBitmapProcState::FixedTileProc tileProcY, SkBitmapProcState::FixedTileLowBitsProc tileLowBitsProcY
#define PREAMBLE_ARG_X          , tileProcX, tileLowBitsProcX
#define PREAMBLE_ARG_Y          , tileProcY, tileLowBitsProcY
#define TILEX_PROCF(fx, max)    SK_USHIFT16(tileProcX(fx) * ((max) + 1))
#define TILEY_PROCF(fy, max)    SK_USHIFT16(tileProcY(fy) * ((max) + 1))
#define TILEX_LOW_BITS(fx, max) tileLowBitsProcX(fx, (max) + 1)
#define TILEY_LOW_BITS(fy, max) tileLowBitsProcY(fy, (max) + 1)
#include "SkBitmapProcState_matrix.h"

static inline U16CPU fixed_clamp(SkFixed x)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    if (x >> 16)
        x = 0xFFFF;
    if (x < 0)
        x = 0;
#else
    if (x >> 16)
    {
        if (x < 0)
            x = 0;
        else
            x = 0xFFFF;
    }
#endif
    return x;
}

static inline U16CPU fixed_repeat(SkFixed x)
{
    return x & 0xFFFF;
}

// Visual Studio 2010 (MSC_VER=1600) optimizes bit-shift code incorrectly.
// See http://code.google.com/p/skia/issues/detail?id=472
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma optimize("", off)
#endif

static inline U16CPU fixed_mirror(SkFixed x)
{
    SkFixed s = x << 15 >> 31;
    // s is FFFFFFFF if we're on an odd interval, or 0 if an even interval
    return (x ^ s) & 0xFFFF;
}

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#pragma optimize("", on)
#endif

static SkBitmapProcState::FixedTileProc choose_tile_proc(unsigned m)
{
    if (SkShader::kClamp_TileMode == m)
        return fixed_clamp;
    if (SkShader::kRepeat_TileMode == m)
        return fixed_repeat;
    SkASSERT(SkShader::kMirror_TileMode == m);
    return fixed_mirror;
}

static inline U16CPU fixed_clamp_lowbits(SkFixed x, int) {
    return (x >> 12) & 0xF;
}

static inline U16CPU fixed_repeat_or_mirrow_lowbits(SkFixed x, int scale) {
    return ((x * scale) >> 12) & 0xF;
}

static SkBitmapProcState::FixedTileLowBitsProc choose_tile_lowbits_proc(unsigned m) {
    if (SkShader::kClamp_TileMode == m) {
        return fixed_clamp_lowbits;
    } else {
        SkASSERT(SkShader::kMirror_TileMode == m ||
                 SkShader::kRepeat_TileMode == m);
        // mirror and repeat have the same behavior for the low bits.
        return fixed_repeat_or_mirrow_lowbits;
    }
}

static inline U16CPU int_clamp(int x, int n) {
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
    if (x >= n)
        x = n - 1;
    if (x < 0)
        x = 0;
#else
    if ((unsigned)x >= (unsigned)n) {
        if (x < 0) {
            x = 0;
        } else {
            x = n - 1;
        }
    }
#endif
    return x;
}

static inline U16CPU int_repeat(int x, int n) {
    return sk_int_mod(x, n);
}

static inline U16CPU int_mirror(int x, int n) {
    x = sk_int_mod(x, 2 * n);
    if (x >= n) {
        x = n + ~(x - n);
    }
    return x;
}

#if 0
static void test_int_tileprocs() {
    for (int i = -8; i <= 8; i++) {
        SkDebugf(" int_mirror(%2d, 3) = %d\n", i, int_mirror(i, 3));
    }
}
#endif

static SkBitmapProcState::IntTileProc choose_int_tile_proc(unsigned tm) {
    if (SkShader::kClamp_TileMode == tm)
        return int_clamp;
    if (SkShader::kRepeat_TileMode == tm)
        return int_repeat;
    SkASSERT(SkShader::kMirror_TileMode == tm);
    return int_mirror;
}

//////////////////////////////////////////////////////////////////////////////

void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count)
{
    int i;

#if	defined(__ARM_HAVE_NEON)
    if (count >= 8) {
        /* SkFixed is 16.16 fixed point */
        SkFixed dx2 = dx+dx;
        SkFixed dx4 = dx2+dx2;
        SkFixed dx8 = dx4+dx4;

        /* now build fx/fx+dx/fx+2dx/fx+3dx */
        SkFixed fx1, fx2, fx3;
        int32x2_t lower, upper;
        int32x4_t lbase, hbase;
        uint16_t *dst16 = (uint16_t *)dst;

        fx1 = fx+dx;
        fx2 = fx1+dx;
        fx3 = fx2+dx;

        /* avoid an 'lbase unitialized' warning */
        lbase = vdupq_n_s32(fx);
        lbase = vsetq_lane_s32(fx1, lbase, 1);
        lbase = vsetq_lane_s32(fx2, lbase, 2);
        lbase = vsetq_lane_s32(fx3, lbase, 3);
        hbase = vaddq_s32(lbase, vdupq_n_s32(dx4));

        /* take upper 16 of each, store, and bump everything */
        do {
            int32x4_t lout, hout;
            uint16x8_t hi16;

            lout = lbase;
            hout = hbase;
            /* gets hi's of all louts then hi's of all houts */
            asm ("vuzpq.16 %q0, %q1" : "+w" (lout), "+w" (hout));
            hi16 = vreinterpretq_u16_s32(hout);
            vst1q_u16(dst16, hi16);

            /* on to the next */
            lbase = vaddq_s32 (lbase, vdupq_n_s32(dx8));
            hbase = vaddq_s32 (hbase, vdupq_n_s32(dx8));
            dst16 += 8;
            count -= 8;
            fx += dx8;
        } while (count >= 8);
        dst = (uint32_t *) dst16;
    }
#else
    for (i = (count >> 2); i > 0; --i)
    {
        *dst++ = pack_two_shorts(fx >> 16, (fx + dx) >> 16);
        fx += dx+dx;
        *dst++ = pack_two_shorts(fx >> 16, (fx + dx) >> 16);
        fx += dx+dx;
    }
    count &= 3;
#endif

    uint16_t* xx = (uint16_t*)dst;
    for (i = count; i > 0; --i) {
        *xx++ = SkToU16(fx >> 16); fx += dx;
    }
}

void decal_filter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count)
{

#if	defined(__ARM_HAVE_NEON)
    if (count >= 8) {
        int32x4_t wide_fx;
        int32x4_t wide_fx2;
        int32x4_t wide_dx8 = vdupq_n_s32(dx*8);

        wide_fx = vdupq_n_s32(fx);
        wide_fx = vsetq_lane_s32(fx+dx, wide_fx, 1);
        wide_fx = vsetq_lane_s32(fx+dx+dx, wide_fx, 2);
        wide_fx = vsetq_lane_s32(fx+dx+dx+dx, wide_fx, 3);

        wide_fx2 = vaddq_s32(wide_fx, vdupq_n_s32(dx+dx+dx+dx));

        while (count >= 8) {
            int32x4_t wide_out;
            int32x4_t wide_out2;

            wide_out = vshlq_n_s32(vshrq_n_s32(wide_fx, 12), 14);
            wide_out = vorrq_s32(wide_out,
            vaddq_s32(vshrq_n_s32(wide_fx,16), vdupq_n_s32(1)));

            wide_out2 = vshlq_n_s32(vshrq_n_s32(wide_fx2, 12), 14);
            wide_out2 = vorrq_s32(wide_out2,
            vaddq_s32(vshrq_n_s32(wide_fx2,16), vdupq_n_s32(1)));

            vst1q_u32(dst, vreinterpretq_u32_s32(wide_out));
            vst1q_u32(dst+4, vreinterpretq_u32_s32(wide_out2));

            dst += 8;
            fx += dx*8;
            wide_fx = vaddq_s32(wide_fx, wide_dx8);
            wide_fx2 = vaddq_s32(wide_fx2, wide_dx8);
            count -= 8;
        }
    }
#endif

    if (count & 1)
    {
        SkASSERT((fx >> (16 + 14)) == 0);
        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;
    }
    while ((count -= 2) >= 0)
    {
        SkASSERT((fx >> (16 + 14)) == 0);
        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;

        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;
    }
}

///////////////////////////////////////////////////////////////////////////////
// stores the same as SCALE, but is cheaper to compute. Also since there is no
// scale, we don't need/have a FILTER version

static void fill_sequential(uint16_t xptr[], int start, int count) {
#if 1
    if (reinterpret_cast<intptr_t>(xptr) & 0x2) {
        *xptr++ = start++;
        count -= 1;
    }
    if (count > 3) {
        uint32_t* xxptr = reinterpret_cast<uint32_t*>(xptr);
        uint32_t pattern0 = PACK_TWO_SHORTS(start + 0, start + 1);
        uint32_t pattern1 = PACK_TWO_SHORTS(start + 2, start + 3);
        start += count & ~3;
        int qcount = count >> 2;
        do {
            *xxptr++ = pattern0;
            pattern0 += 0x40004;
            *xxptr++ = pattern1;
            pattern1 += 0x40004;
        } while (--qcount != 0);
        xptr = reinterpret_cast<uint16_t*>(xxptr);
        count &= 3;
    }
    while (--count >= 0) {
        *xptr++ = start++;
    }
#else
    for (int i = 0; i < count; i++) {
        *xptr++ = start++;
    }
#endif
}

static int nofilter_trans_preamble(const SkBitmapProcState& s, uint32_t** xy,
                                   int x, int y) {
    SkPoint pt;
    s.fInvProc(*s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
               SkIntToScalar(y) + SK_ScalarHalf, &pt);
    **xy = s.fIntTileProcY(SkScalarToFixed(pt.fY) >> 16,
                           s.fBitmap->height());
    *xy += 1;   // bump the ptr
    // return our starting X position
    return SkScalarToFixed(pt.fX) >> 16;
}

static void clampx_nofilter_trans(const SkBitmapProcState& s,
                                  uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~SkMatrix::kTranslate_Mask) == 0);

    int xpos = nofilter_trans_preamble(s, &xy, x, y);
    const int width = s.fBitmap->width();    
    if (1 == width) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }
    
    uint16_t* xptr = reinterpret_cast<uint16_t*>(xy);
    int n;

    // fill before 0 as needed
    if (xpos < 0) {
        n = -xpos;
        if (n > count) {
            n = count;
        }
        memset(xptr, 0, n * sizeof(uint16_t));
        count -= n;
        if (0 == count) {
            return;
        }
        xptr += n;
        xpos = 0;
    }

    // fill in 0..width-1 if needed
    if (xpos < width) {
        n = width - xpos;
        if (n > count) {
            n = count;
        }
        fill_sequential(xptr, xpos, n);
        count -= n;
        if (0 == count) {
            return;
        }
        xptr += n;
    }

    // fill the remaining with the max value
    sk_memset16(xptr, width - 1, count);
}

static void repeatx_nofilter_trans(const SkBitmapProcState& s,
                                   uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~SkMatrix::kTranslate_Mask) == 0);

    int xpos = nofilter_trans_preamble(s, &xy, x, y);
    const int width = s.fBitmap->width();    
    if (1 == width) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    uint16_t* xptr = reinterpret_cast<uint16_t*>(xy);
    int start = sk_int_mod(xpos, width);
    int n = width - start;
    if (n > count) {
        n = count;
    }
    fill_sequential(xptr, start, n);
    xptr += n;
    count -= n;

    while (count >= width) {
        fill_sequential(xptr, 0, width);
        xptr += width;
        count -= width;
    }

    if (count > 0) {
        fill_sequential(xptr, 0, count);
    }
}

static void fill_backwards(uint16_t xptr[], int pos, int count) {
    for (int i = 0; i < count; i++) {
        SkASSERT(pos >= 0);
        xptr[i] = pos--;
    }
}

static void mirrorx_nofilter_trans(const SkBitmapProcState& s,
                                   uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~SkMatrix::kTranslate_Mask) == 0);

    int xpos = nofilter_trans_preamble(s, &xy, x, y);
    const int width = s.fBitmap->width();    
    if (1 == width) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    uint16_t* xptr = reinterpret_cast<uint16_t*>(xy);
    // need to know our start, and our initial phase (forward or backward)
    bool forward;
    int n;
    int start = sk_int_mod(xpos, 2 * width);
    if (start >= width) {
        start = width + ~(start - width);
        forward = false;
        n = start + 1;  // [start .. 0]
    } else {
        forward = true;
        n = width - start;  // [start .. width)
    }
    if (n > count) {
        n = count;
    }
    if (forward) {
        fill_sequential(xptr, start, n);
    } else {
        fill_backwards(xptr, start, n);
    }
    forward = !forward;
    xptr += n;
    count -= n;
    
    while (count >= width) {
        if (forward) {
            fill_sequential(xptr, 0, width);
        } else {
            fill_backwards(xptr, width - 1, width);
        }
        forward = !forward;
        xptr += width;
        count -= width;
    }
    
    if (count > 0) {
        if (forward) {
            fill_sequential(xptr, 0, count);
        } else {
            fill_backwards(xptr, width - 1, count);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkBitmapProcState::MatrixProc
SkBitmapProcState::chooseMatrixProc(bool trivial_matrix) {
//    test_int_tileprocs();
    // check for our special case when there is no scale/affine/perspective
    if (trivial_matrix) {
        SkASSERT(!fDoFilter);
        fIntTileProcY = choose_int_tile_proc(fTileModeY);
        switch (fTileModeX) {
            case SkShader::kClamp_TileMode:
                return clampx_nofilter_trans;
            case SkShader::kRepeat_TileMode:
                return repeatx_nofilter_trans;
            case SkShader::kMirror_TileMode:
                return mirrorx_nofilter_trans;
        }
    }
    
    int index = 0;
    if (fDoFilter) {
        index = 1;
    }
    if (fInvType & SkMatrix::kPerspective_Mask) {
        index += 4;
    } else if (fInvType & SkMatrix::kAffine_Mask) {
        index += 2;
    }
    
    if (SkShader::kClamp_TileMode == fTileModeX &&
        SkShader::kClamp_TileMode == fTileModeY)
    {
        // clamp gets special version of filterOne
        fFilterOneX = SK_Fixed1;
        fFilterOneY = SK_Fixed1;
        return ClampX_ClampY_Procs[index];
    }
    
    // all remaining procs use this form for filterOne
    fFilterOneX = SK_Fixed1 / fBitmap->width();
    fFilterOneY = SK_Fixed1 / fBitmap->height();
    
    if (SkShader::kRepeat_TileMode == fTileModeX &&
        SkShader::kRepeat_TileMode == fTileModeY)
    {
        return RepeatX_RepeatY_Procs[index];
    }
    
    fTileProcX = choose_tile_proc(fTileModeX);
    fTileProcY = choose_tile_proc(fTileModeY);
    fTileLowBitsProcX = choose_tile_lowbits_proc(fTileModeX);
    fTileLowBitsProcY = choose_tile_lowbits_proc(fTileModeY);
    return GeneralXY_Procs[index];
}

