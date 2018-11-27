/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// The copyright below was added in 2009, but I see no record of moto contributions...?

/* NEON optimized code (C) COPYRIGHT 2009 Motorola
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapProcState.h"
#include "SkBitmapProcState_utils.h"
#include "SkShader.h"
#include "SkTo.h"
#include "SkUtils.h"

// When not filtering, we store 32-bit y, 16-bit x, 16-bit x, 16-bit x, ...
// When filtering we write out 32-bit encodings, pairing 14.4 x0 with 14-bit x1.

// The clamp routines may try to fall into one of these unclamped decal fast-paths.
// (Only clamp works in the right coordinate space to check for decal.)
static void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    for (; count >= 2; count -= 2) {
        *dst++ = pack_two_shorts( (fx +  0) >> 16,
                                  (fx + dx) >> 16);
        fx += dx+dx;
    }

    auto xx = (uint16_t*)dst;
    while (count --> 0) {
        *xx++ = SkToU16(fx >> 16);
        fx += dx;
    }
}

// A generic implementation for unfiltered scale+translate, templated on tiling method.
template <unsigned (*tile)(SkFixed, int), bool tryDecal>
static void nofilter_scale(const SkBitmapProcState& s,
                           uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);

    // Write out our 32-bit y, and get our intial fx.
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        *xy++ = tile(mapper.fixedY(), s.fPixmap.height() - 1);
        fx = mapper.fractionalIntX();
    }

    const unsigned maxX = s.fPixmap.width() - 1;
    if (0 == maxX) {
        // If width == 1, all the x-values must refer to that pixel, and must be zero.
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    const SkFractionalInt dx = s.fInvSxFractionalInt;

    if (tryDecal) {
        const SkFixed fixedFx = SkFractionalIntToFixed(fx);
        const SkFixed fixedDx = SkFractionalIntToFixed(dx);

        if (can_truncate_to_fixed_for_decal(fixedFx, fixedDx, count, maxX)) {
            decal_nofilter_scale(xy, fixedFx, fixedDx, count);
            return;
        }
    }

    // Remember, each x-coordinate is 16-bit.
    for (; count >= 2; count -= 2) {
        *xy++ = pack_two_shorts(tile(SkFractionalIntToFixed(fx     ), maxX),
                                tile(SkFractionalIntToFixed(fx + dx), maxX));
        fx += dx+dx;
    }

    auto xx = (uint16_t*)xy;
    while (count --> 0) {
        *xx++ = tile(SkFractionalIntToFixed(fx), maxX);
        fx += dx;
    }
}

// Clamp/Clamp and Repeat/Repeat have NEON or portable implementations.
// TODO: switch SkBitmapProcState_matrix.h to templates instead of #defines and multiple includes?

#if defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>

    // TODO: this is a fine drop-in for decal_nofilter_scale() generally.
    static void decal_nofilter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
        if (count >= 8) {
            // SkFixed is 16.16 fixed point
            SkFixed dx8 = dx * 8;
            int32x4_t vdx8 = vdupq_n_s32(dx8);

            // setup lbase and hbase
            int32x4_t lbase, hbase;
            lbase = vdupq_n_s32(fx);
            lbase = vsetq_lane_s32(fx + dx, lbase, 1);
            lbase = vsetq_lane_s32(fx + dx + dx, lbase, 2);
            lbase = vsetq_lane_s32(fx + dx + dx + dx, lbase, 3);
            hbase = lbase + vdupq_n_s32(4 * dx);

            do {
                // store the upper 16 bits
                vst1q_u32(dst, vreinterpretq_u32_s16(
                    vuzpq_s16(vreinterpretq_s16_s32(lbase), vreinterpretq_s16_s32(hbase)).val[1]
                ));

                // on to the next group of 8
                lbase += vdx8;
                hbase += vdx8;
                dst += 4; // we did 8 elements but the result is twice smaller
                count -= 8;
                fx += dx8;
            } while (count >= 8);
        }

        uint16_t* xx = (uint16_t*)dst;
        for (int i = count; i > 0; --i) {
            *xx++ = SkToU16(fx >> 16); fx += dx;
        }
    }

    static void decal_filter_scale_neon(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
        if (count >= 8) {
            SkFixed dx8 = dx * 8;
            int32x4_t vdx8 = vdupq_n_s32(dx8);

            int32x4_t wide_fx, wide_fx2;
            wide_fx = vdupq_n_s32(fx);
            wide_fx = vsetq_lane_s32(fx + dx, wide_fx, 1);
            wide_fx = vsetq_lane_s32(fx + dx + dx, wide_fx, 2);
            wide_fx = vsetq_lane_s32(fx + dx + dx + dx, wide_fx, 3);

            wide_fx2 = vaddq_s32(wide_fx, vdupq_n_s32(4 * dx));

            while (count >= 8) {
                int32x4_t wide_out;
                int32x4_t wide_out2;

                wide_out = vshlq_n_s32(vshrq_n_s32(wide_fx, 12), 14);
                wide_out = wide_out | (vshrq_n_s32(wide_fx,16) + vdupq_n_s32(1));

                wide_out2 = vshlq_n_s32(vshrq_n_s32(wide_fx2, 12), 14);
                wide_out2 = wide_out2 | (vshrq_n_s32(wide_fx2,16) + vdupq_n_s32(1));

                vst1q_u32(dst, vreinterpretq_u32_s32(wide_out));
                vst1q_u32(dst+4, vreinterpretq_u32_s32(wide_out2));

                dst += 8;
                fx += dx8;
                wide_fx += vdx8;
                wide_fx2 += vdx8;
                count -= 8;
            }
        }

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

    // TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
    static inline int16x8_t sbpsm_clamp_tile8(int32x4_t low, int32x4_t high, unsigned max) {
        int16x8_t res;

        // get the hi 16s of all those 32s
        res = vuzpq_s16(vreinterpretq_s16_s32(low), vreinterpretq_s16_s32(high)).val[1];

        // clamp
        res = vmaxq_s16(res, vdupq_n_s16(0));
        res = vminq_s16(res, vdupq_n_s16(max));

        return res;
    }

    // TILEX_PROCF(fx, max)    SkClampMax((fx) >> 16, max)
    static inline int32x4_t sbpsm_clamp_tile4(int32x4_t f, unsigned max) {
        int32x4_t res;

        // get the hi 16s of all those 32s
        res = vshrq_n_s32(f, 16);

        // clamp
        res = vmaxq_s32(res, vdupq_n_s32(0));
        res = vminq_s32(res, vdupq_n_s32(max));

        return res;
    }

    // EXTRACT_LOW_BITS(fy, max)         (((fy) >> 12) & 0xF)
    static inline int32x4_t sbpsm_clamp_tile4_low_bits(int32x4_t fx) {
        int32x4_t ret;

        ret = vshrq_n_s32(fx, 12);

        /* We don't need the mask below because the caller will
         * overwrite the non-masked bits
         */
        //ret = vandq_s32(ret, vdupq_n_s32(0xF));

        return ret;
    }

    // TILEX_PROCF(fx, max) (((fx)&0xFFFF)*((max)+1)>> 16)
    static inline int16x8_t sbpsm_repeat_tile8(int32x4_t low, int32x4_t high, unsigned max) {
        uint16x8_t res;
        uint32x4_t tmpl, tmph;

        // get the lower 16 bits
        res = vuzpq_u16(vreinterpretq_u16_s32(low), vreinterpretq_u16_s32(high)).val[0];

        // bare multiplication, not SkFixedMul
        tmpl = vmull_u16(vget_low_u16(res), vdup_n_u16(max+1));
        tmph = vmull_u16(vget_high_u16(res), vdup_n_u16(max+1));

        // extraction of the 16 upper bits
        res = vuzpq_u16(vreinterpretq_u16_u32(tmpl), vreinterpretq_u16_u32(tmph)).val[1];

        return vreinterpretq_s16_u16(res);
    }

    // TILEX_PROCF(fx, max) (((fx)&0xFFFF)*((max)+1)>> 16)
    static inline int32x4_t sbpsm_repeat_tile4(int32x4_t f, unsigned max) {
        uint16x4_t res;
        uint32x4_t tmp;

        // get the lower 16 bits
        res = vmovn_u32(vreinterpretq_u32_s32(f));

        // bare multiplication, not SkFixedMul
        tmp = vmull_u16(res, vdup_n_u16(max+1));

        // extraction of the 16 upper bits
        tmp = vshrq_n_u32(tmp, 16);

        return vreinterpretq_s32_u32(tmp);
    }

    // EXTRACT_LOW_BITS(fx, max)         ((((fx) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
    static inline int32x4_t sbpsm_repeat_tile4_low_bits(int32x4_t fx, unsigned max) {
        uint16x4_t res;
        uint32x4_t tmp;
        int32x4_t ret;

        // get the lower 16 bits
        res = vmovn_u32(vreinterpretq_u32_s32(fx));

        // bare multiplication, not SkFixedMul
        tmp = vmull_u16(res, vdup_n_u16(max + 1));

        // shift and mask
        ret = vshrq_n_s32(vreinterpretq_s32_u32(tmp), 12);

        /* We don't need the mask below because the caller will
         * overwrite the non-masked bits
         */
        //ret = vandq_s32(ret, vdupq_n_s32(0xF));

        return ret;
    }

    #define MAKENAME(suffix)                ClampX_ClampY ## suffix ## _neon
    #define TILEX_PROCF(fx, max)            SkClampMax((fx) >> 16, max)
    #define TILEY_PROCF(fy, max)            SkClampMax((fy) >> 16, max)
    #define TILEX_PROCF_NEON8(l, h, max)    sbpsm_clamp_tile8(l, h, max)
    #define TILEY_PROCF_NEON8(l, h, max)    sbpsm_clamp_tile8(l, h, max)
    #define TILEX_PROCF_NEON4(fx, max)      sbpsm_clamp_tile4(fx, max)
    #define TILEY_PROCF_NEON4(fy, max)      sbpsm_clamp_tile4(fy, max)
    #define EXTRACT_LOW_BITS(v, max)        (((v) >> 12) & 0xF)
    #define EXTRACT_LOW_BITS_NEON4(v, max)  sbpsm_clamp_tile4_low_bits(v)
    #define CHECK_FOR_DECAL
    #include "SkBitmapProcState_matrix_neon.h"

    #define MAKENAME(suffix)                RepeatX_RepeatY ## suffix ## _neon
    #define TILEX_PROCF(fx, max)            SK_USHIFT16(((fx) & 0xFFFF) * ((max) + 1))
    #define TILEY_PROCF(fy, max)            SK_USHIFT16(((fy) & 0xFFFF) * ((max) + 1))
    #define TILEX_PROCF_NEON8(l, h, max)    sbpsm_repeat_tile8(l, h, max)
    #define TILEY_PROCF_NEON8(l, h, max)    sbpsm_repeat_tile8(l, h, max)
    #define TILEX_PROCF_NEON4(fx, max)      sbpsm_repeat_tile4(fx, max)
    #define TILEY_PROCF_NEON4(fy, max)      sbpsm_repeat_tile4(fy, max)
    #define EXTRACT_LOW_BITS(v, max)        ((((v) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
    #define EXTRACT_LOW_BITS_NEON4(v, max)  sbpsm_repeat_tile4_low_bits(v, max)
    #include "SkBitmapProcState_matrix_neon.h"

#else
    static unsigned clamp(SkFixed fx, int max) {
        return SkClampMax(fx >> 16, max);
    }

    #define MAKENAME(suffix)         ClampX_ClampY ## suffix
    #define TILE_PROCF               clamp
    #define EXTRACT_LOW_BITS(v, max) (((v) >> 12) & 0xF)
    #define CHECK_FOR_DECAL
    #include "SkBitmapProcState_matrix.h"  // will create ClampX_ClampY_filter_scale.

    // This and ClampX_ClampY_filter_scale() are both extern for now so that opts_check_x86.cpp
    // can identify and replace them.  TODO: clean up when opts_check_x86.cpp is gone.
    void ClampX_ClampY_nofilter_scale(const SkBitmapProcState& s,
                                      uint32_t xy[], int count, int x, int y) {
        nofilter_scale<clamp, true>(s, xy, count, x,y);
    }

    static const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs[] = {
        ClampX_ClampY_nofilter_scale,
        ClampX_ClampY_filter_scale,
    };


    static unsigned repeat(SkFixed fx, int max) {
        SkASSERT(max < 65535);
        return SK_USHIFT16((unsigned)(fx & 0xFFFF) * (max + 1));
    }

    #define MAKENAME(suffix)         RepeatX_RepeatY ## suffix
    #define TILE_PROCF               repeat
    #define EXTRACT_LOW_BITS(v, max) (((v * (max + 1)) >> 12) & 0xF)
    #include "SkBitmapProcState_matrix.h"  // will create RepeatX_RepeatY_filter_scale.

    static const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs[] = {
        nofilter_scale<repeat, false>,
        RepeatX_RepeatY_filter_scale,
    };
#endif

static unsigned mirror(SkFixed fx, int max) {
    SkASSERT(max < 65535);
    // s is 0xFFFFFFFF if we're on an odd interval, or 0 if an even interval
    SkFixed s = SkLeftShift(fx, 15) >> 31;

    // This should be exactly the same as repeat(fx ^ s, max) from here on.
    return SK_USHIFT16( ((fx ^ s) & 0xFFFF) * (max + 1) );
}

#define MAKENAME(suffix)         MirrorX_MirrorY ## suffix
#define TILE_PROCF               mirror
#define EXTRACT_LOW_BITS(v, max) (((v * (max + 1)) >> 12) & 0xF)
#include "SkBitmapProcState_matrix.h"  // will create MirrorX_MirrorY_filter_scale.

static const SkBitmapProcState::MatrixProc MirrorX_MirrorY_Procs[] = {
    nofilter_scale<mirror, false>,
    MirrorX_MirrorY_filter_scale,
};


///////////////////////////////////////////////////////////////////////////////
// This next chunk has some specializations for unfiltered translate-only matrices.

static inline U16CPU int_clamp(int x, int n) {
    if (x <  0) { x = 0; }
    if (x >= n) { x = n - 1; }
    return x;
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

static void fill_sequential(uint16_t xptr[], int pos, int count) {
    while (count --> 0) {
        *xptr++ = pos++;
    }
}

static void fill_backwards(uint16_t xptr[], int pos, int count) {
    while (count --> 0) {
        SkASSERT(pos >= 0);
        *xptr++ = pos--;
    }
}

static void clampx_nofilter_trans(const SkBitmapProcState& s,
                                  uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~SkMatrix::kTranslate_Mask) == 0);

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    *xy++ = int_clamp(mapper.intY(), s.fPixmap.height());
    int xpos = mapper.intX();

    const int width = s.fPixmap.width();
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

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    *xy++ = int_repeat(mapper.intY(), s.fPixmap.height());
    int xpos = mapper.intX();

    const int width = s.fPixmap.width();
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

static void mirrorx_nofilter_trans(const SkBitmapProcState& s,
                                   uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~SkMatrix::kTranslate_Mask) == 0);

    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    *xy++ = int_mirror(mapper.intY(), s.fPixmap.height());
    int xpos = mapper.intX();

    const int width = s.fPixmap.width();
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
// The main entry point to the file, choosing between everything above.

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc(bool translate_only_matrix) {
    SkASSERT(fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(fTileModeX == fTileModeY);
    SkASSERT(fTileModeX != SkShader::kDecal_TileMode);

    // Check for our special case translate methods when there is no scale/affine/perspective.
    if (translate_only_matrix && kNone_SkFilterQuality == fFilterQuality) {
        switch (fTileModeX) {
            default: SkASSERT(false);
            case SkShader::kClamp_TileMode:  return  clampx_nofilter_trans;
            case SkShader::kRepeat_TileMode: return repeatx_nofilter_trans;
            case SkShader::kMirror_TileMode: return mirrorx_nofilter_trans;
        }
    }

    // The arrays are all [ nofilter, filter ].
    int index = fFilterQuality > kNone_SkFilterQuality ? 1 : 0;

    if (fTileModeX == SkShader::kClamp_TileMode) {
        // clamp gets special version of filterOne, working in non-normalized space (allowing decal)
        fFilterOneX = SK_Fixed1;
        fFilterOneY = SK_Fixed1;
    #if defined(SK_ARM_HAS_NEON)
        return ClampX_ClampY_Procs_neon[index];
    #else
        return ClampX_ClampY_Procs[index];
    #endif
    }

    // all remaining procs use this form for filterOne, putting them into normalized space.
    fFilterOneX = SK_Fixed1 / fPixmap.width();
    fFilterOneY = SK_Fixed1 / fPixmap.height();

    if (fTileModeX == SkShader::kRepeat_TileMode) {
    #if defined(SK_ARM_HAS_NEON)
        return RepeatX_RepeatY_Procs_neon[index];
    #else
        return RepeatX_RepeatY_Procs[index];
    #endif
    }

    return MirrorX_MirrorY_Procs[index];
}
