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
#include "SkPerspIter.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkUtilsArm.h"
#include "SkBitmapProcState_utils.h"

void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count);
void decal_filter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count);

#include "SkBitmapProcState_matrix_template.h"

///////////////////////////////////////////////////////////////////////////////

// Compile neon code paths if needed
#if defined(SK_ARM_HAS_NEON)

// These are defined in src/opts/SkBitmapProcState_matrixProcs_neon.cpp
extern const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs_neon[];

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
    NoFilterProc_Persp<ClampTileProcs>,
    ClampX_ClampY_filter_persp
};

#endif

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

static inline U16CPU int_clamp(int x, int n) {
    if (x >= n) {
        x = n - 1;
    }
    if (x < 0) {
        x = 0;
    }
    return x;
}

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc(bool trivial_matrix) {
    SkASSERT(fTileModeX == SkShader::kClamp_TileMode);
    SkASSERT(fTileModeY == SkShader::kClamp_TileMode);

    // check for our special case when there is no scale/affine/perspective
    if (trivial_matrix && kNone_SkFilterQuality == fFilterQuality) {
        fIntTileProcY = int_clamp;
        return clampx_nofilter_trans;
    }

    int index = 0;
    if (fFilterQuality != kNone_SkFilterQuality) {
        index = 1;
    }
    if (fInvType & SkMatrix::kPerspective_Mask) {
        index += 4;
    } else if (fInvType & SkMatrix::kAffine_Mask) {
        index += 2;
    }

    // clamp gets special version of filterOne
    fFilterOneX = SK_Fixed1;
    fFilterOneY = SK_Fixed1;
    return SK_ARM_NEON_WRAP(ClampX_ClampY_Procs)[index];
}
