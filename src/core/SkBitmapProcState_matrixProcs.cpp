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
    // These are defined in src/opts/SkBitmapProcState_matrixProcs_neon.cpp
    extern const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs_neon[];
    extern const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs_neon[];
#else
    static void decal_filter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
        while (count --> 0) {
            SkASSERT((fx >> (16 + 14)) == 0);
            *dst++ = (fx >> 12 << 14) | ((fx >> 16) + 1);
            fx += dx;
        }
    }

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
// The main entry point to the file, choosing between everything above.

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc() {
    SkASSERT(fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(fTileModeX == fTileModeY);
    SkASSERT(fTileModeX != SkShader::kDecal_TileMode);

    // The arrays are all [ nofilter, filter ].
    int index = fFilterQuality > kNone_SkFilterQuality ? 1 : 0;

    if (SkShader::kClamp_TileMode == fTileModeX) {
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

    if (SkShader::kRepeat_TileMode == fTileModeX) {
    #if defined(SK_ARM_HAS_NEON)
        return RepeatX_RepeatY_Procs_neon[index];
    #else
        return RepeatX_RepeatY_Procs[index];
    #endif
    }

    return MirrorX_MirrorY_Procs[index];
}
