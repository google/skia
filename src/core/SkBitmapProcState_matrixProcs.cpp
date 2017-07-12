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
#include "SkShader.h"
#include "SkUtils.h"
#include "SkUtilsArm.h"
#include "SkBitmapProcState_utils.h"

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

#include "SkBitmapProcState_matrix_template.h"

///////////////////////////////////////////////////////////////////////////////

// Compile neon code paths if needed
#if defined(SK_ARM_HAS_NEON)

// These are defined in src/opts/SkBitmapProcState_matrixProcs_neon.cpp
extern const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs_neon[];
extern const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs_neon[];

#endif // defined(SK_ARM_HAS_NEON)

// Compile non-neon code path if needed
#if !defined(SK_ARM_HAS_NEON)
#define MAKENAME(suffix)         ClampX_ClampY ## suffix
#define TILEX_PROCF(fx, max)     SkClampMax((fx) >> 16, max)
#define TILEY_PROCF(fy, max)     SkClampMax((fy) >> 16, max)
#define EXTRACT_LOW_BITS(v, max) (((v) >> 12) & 0xF)
#define CHECK_FOR_DECAL
#include "SkBitmapProcState_matrix.h"

struct ClampTileProcs {
    static unsigned X(const SkBitmapProcState&, SkFixed fx, int max) {
        return SkClampMax(fx >> 16, max);
    }
    static unsigned Y(const SkBitmapProcState&, SkFixed fy, int max) {
        return SkClampMax(fy >> 16, max);
    }
};

// Referenced in opts_check_x86.cpp
void ClampX_ClampY_nofilter_scale(const SkBitmapProcState& s, uint32_t xy[],
                                  int count, int x, int y) {
    return NoFilterProc_Scale<ClampTileProcs, true>(s, xy, count, x, y);
}
void ClampX_ClampY_nofilter_affine(const SkBitmapProcState& s, uint32_t xy[],
                                  int count, int x, int y) {
    return NoFilterProc_Affine<ClampTileProcs>(s, xy, count, x, y);
}

static SkBitmapProcState::MatrixProc ClampX_ClampY_Procs[] = {
    // only clamp lives in the right coord space to check for decal
    ClampX_ClampY_nofilter_scale,
    ClampX_ClampY_filter_scale,
    ClampX_ClampY_nofilter_affine,
    ClampX_ClampY_filter_affine,
};

#define MAKENAME(suffix)         RepeatX_RepeatY ## suffix
#define TILEX_PROCF(fx, max)     SK_USHIFT16((unsigned)((fx) & 0xFFFF) * ((max) + 1))
#define TILEY_PROCF(fy, max)     SK_USHIFT16((unsigned)((fy) & 0xFFFF) * ((max) + 1))
#define EXTRACT_LOW_BITS(v, max) (((unsigned)((v) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
#include "SkBitmapProcState_matrix.h"

struct RepeatTileProcs {
    static unsigned X(const SkBitmapProcState&, SkFixed fx, int max) {
        SkASSERT(max < 65535);
        return SK_USHIFT16((unsigned)((fx) & 0xFFFF) * ((max) + 1));
    }
    static unsigned Y(const SkBitmapProcState&, SkFixed fy, int max) {
        SkASSERT(max < 65535);
        return SK_USHIFT16((unsigned)((fy) & 0xFFFF) * ((max) + 1));
    }
};

static SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs[] = {
    NoFilterProc_Scale<RepeatTileProcs, false>,
    RepeatX_RepeatY_filter_scale,
    NoFilterProc_Affine<RepeatTileProcs>,
    RepeatX_RepeatY_filter_affine,
};
#endif

#define MAKENAME(suffix)        GeneralXY ## suffix
#define PREAMBLE(state)         SkBitmapProcState::FixedTileProc tileProcX = (state).fTileProcX; (void) tileProcX; \
                                SkBitmapProcState::FixedTileProc tileProcY = (state).fTileProcY; (void) tileProcY;
#define PREAMBLE_PARAM_X        , SkBitmapProcState::FixedTileProc tileProcX
#define PREAMBLE_PARAM_Y        , SkBitmapProcState::FixedTileProc tileProcY
#define PREAMBLE_ARG_X          , tileProcX
#define PREAMBLE_ARG_Y          , tileProcY
#define TILEX_PROCF(fx, max)    SK_USHIFT16(tileProcX(fx) * ((max) + 1))
#define TILEY_PROCF(fy, max)    SK_USHIFT16(tileProcY(fy) * ((max) + 1))
#define EXTRACT_LOW_BITS(v, max) (((v * (max + 1)) >> 12) & 0xF)
#include "SkBitmapProcState_matrix.h"

struct GeneralTileProcs {
    static unsigned X(const SkBitmapProcState& s, SkFixed fx, int max) {
        return SK_USHIFT16(s.fTileProcX(fx) * ((max) + 1));
    }
    static unsigned Y(const SkBitmapProcState& s, SkFixed fy, int max) {
        return SK_USHIFT16(s.fTileProcY(fy) * ((max) + 1));
    }
};

static SkBitmapProcState::MatrixProc GeneralXY_Procs[] = {
    NoFilterProc_Scale<GeneralTileProcs, false>,
    GeneralXY_filter_scale,
    NoFilterProc_Affine<GeneralTileProcs>,
    GeneralXY_filter_affine,
};

///////////////////////////////////////////////////////////////////////////////

static inline U16CPU fixed_clamp(SkFixed x) {
    if (x < 0) {
        x = 0;
    }
    if (x >> 16) {
        x = 0xFFFF;
    }
    return x;
}

static inline U16CPU fixed_repeat(SkFixed x) {
    return x & 0xFFFF;
}

static inline U16CPU fixed_mirror(SkFixed x) {
    SkFixed s = SkLeftShift(x, 15) >> 31;
    // s is FFFFFFFF if we're on an odd interval, or 0 if an even interval
    return (x ^ s) & 0xFFFF;
}

static SkBitmapProcState::FixedTileProc choose_tile_proc(unsigned m) {
    if (SkShader::kClamp_TileMode == m) {
        return fixed_clamp;
    }
    if (SkShader::kRepeat_TileMode == m) {
        return fixed_repeat;
    }
    SkASSERT(SkShader::kMirror_TileMode == m);
    return fixed_mirror;
}

static inline U16CPU int_clamp(int x, int n) {
    if (x >= n) {
        x = n - 1;
    }
    if (x < 0) {
        x = 0;
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

void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    int i;

    for (i = (count >> 2); i > 0; --i) {
        *dst++ = pack_two_shorts(fx >> 16, (fx + dx) >> 16);
        fx += dx+dx;
        *dst++ = pack_two_shorts(fx >> 16, (fx + dx) >> 16);
        fx += dx+dx;
    }
    count &= 3;

    uint16_t* xx = (uint16_t*)dst;
    for (i = count; i > 0; --i) {
        *xx++ = SkToU16(fx >> 16); fx += dx;
    }
}

void decal_filter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    if (count & 1) {
        SkASSERT((fx >> (16 + 14)) == 0);
        *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
        fx += dx;
    }
    while ((count -= 2) >= 0) {
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
    const SkBitmapProcStateAutoMapper mapper(s, x, y);
    **xy = s.fIntTileProcY(mapper.intY(), s.fPixmap.height());
    *xy += 1;   // bump the ptr
    // return our starting X position
    return mapper.intX();
}

static void clampx_nofilter_trans(const SkBitmapProcState& s,
                                  uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~SkMatrix::kTranslate_Mask) == 0);

    int xpos = nofilter_trans_preamble(s, &xy, x, y);
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

    int xpos = nofilter_trans_preamble(s, &xy, x, y);
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

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc(bool trivial_matrix) {
    SkASSERT((fInvType & SkMatrix::kPerspective_Mask) == 0);

//    test_int_tileprocs();
    // check for our special case when there is no scale/affine/perspective
    if (trivial_matrix && kNone_SkFilterQuality == fFilterQuality) {
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
    if (fFilterQuality != kNone_SkFilterQuality) {
        index = 1;
    }
    if (fInvType & SkMatrix::kAffine_Mask) {
        index += 2;
    }

    if (SkShader::kClamp_TileMode == fTileModeX && SkShader::kClamp_TileMode == fTileModeY) {
        // clamp gets special version of filterOne
        fFilterOneX = SK_Fixed1;
        fFilterOneY = SK_Fixed1;
        return SK_ARM_NEON_WRAP(ClampX_ClampY_Procs)[index];
    }

    // all remaining procs use this form for filterOne
    fFilterOneX = SK_Fixed1 / fPixmap.width();
    fFilterOneY = SK_Fixed1 / fPixmap.height();

    if (SkShader::kRepeat_TileMode == fTileModeX && SkShader::kRepeat_TileMode == fTileModeY) {
        return SK_ARM_NEON_WRAP(RepeatX_RepeatY_Procs)[index];
    }

    fTileProcX = choose_tile_proc(fTileModeX);
    fTileProcY = choose_tile_proc(fTileModeY);
    return GeneralXY_Procs[index];
}
