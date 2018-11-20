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
template <typename TileProc, bool tryDecal>
static void nofilter_scale(const SkBitmapProcState& s,
                           uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);

    // Write out our 32-bit y, and get our intial fx.
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        *xy++ = TileProc::Y(s, mapper.fixedY(), s.fPixmap.height() - 1);
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
        *xy++ = pack_two_shorts(TileProc::X(s, SkFractionalIntToFixed(fx     ), maxX),
                                TileProc::X(s, SkFractionalIntToFixed(fx + dx), maxX));
        fx += dx+dx;
    }

    auto xx = (uint16_t*)xy;
    while (count --> 0) {
        *xx++ = TileProc::X(s, SkFractionalIntToFixed(fx), maxX);
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

    #define MAKENAME(suffix)         ClampX_ClampY ## suffix
    #define TILEX_PROCF(fx, max)     SkClampMax((fx) >> 16, max)
    #define TILEY_PROCF(fy, max)     SkClampMax((fy) >> 16, max)
    #define EXTRACT_LOW_BITS(v, max) (((v) >> 12) & 0xF)
    #define CHECK_FOR_DECAL
    #include "SkBitmapProcState_matrix.h"  // will create ClampX_ClampY_filter_scale.

    struct ClampTileProcs {
        static unsigned X(const SkBitmapProcState&, SkFixed fx, int max) {
            return SkClampMax(fx >> 16, max);
        }
        static unsigned Y(const SkBitmapProcState&, SkFixed fy, int max) {
            return SkClampMax(fy >> 16, max);
        }
    };

    // This and ClampX_ClampY_filter_scale() are both extern for now so that opts_check_x86.cpp
    // can identify and replace them.  TODO: clean up when opts_check_x86.cpp is gone.
    void ClampX_ClampY_nofilter_scale(const SkBitmapProcState& s,
                                      uint32_t xy[], int count, int x, int y) {
        nofilter_scale<ClampTileProcs, true>(s, xy, count, x,y);
    }

    static SkBitmapProcState::MatrixProc ClampX_ClampY_Procs[] = {
        ClampX_ClampY_nofilter_scale,
        ClampX_ClampY_filter_scale,
    };

    #define MAKENAME(suffix)         RepeatX_RepeatY ## suffix
    #define TILEX_PROCF(fx, max)     SK_USHIFT16((unsigned)((fx) & 0xFFFF) * ((max) + 1))
    #define TILEY_PROCF(fy, max)     SK_USHIFT16((unsigned)((fy) & 0xFFFF) * ((max) + 1))
    #define EXTRACT_LOW_BITS(v, max) (((unsigned)((v) & 0xFFFF) * ((max) + 1) >> 12) & 0xF)
    #include "SkBitmapProcState_matrix.h"  // will create RepeatX_RepeatY_filter_scale.

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
        nofilter_scale<RepeatTileProcs, false>,
        RepeatX_RepeatY_filter_scale,
    };
#endif

// If not Clamp/Clamp or Repeat/Repeat, everyone falls back to this general-case path.
// TODO: this really only handles Mirror/Mirror today.  Heterogenous tiling doesn't get here.

#define MAKENAME(suffix)        GeneralXY ## suffix
#define PREAMBLE(state)         SkBitmapProcState::FixedTileProc tileProcX = (state).fTileProcX; \
                                (void) tileProcX;                                                \
                                SkBitmapProcState::FixedTileProc tileProcY = (state).fTileProcY; \
                                (void) tileProcY;
#define PREAMBLE_PARAM_X        , SkBitmapProcState::FixedTileProc tileProcX
#define PREAMBLE_PARAM_Y        , SkBitmapProcState::FixedTileProc tileProcY
#define PREAMBLE_ARG_X          , tileProcX
#define PREAMBLE_ARG_Y          , tileProcY
#define TILEX_PROCF(fx, max)    SK_USHIFT16(tileProcX(fx) * ((max) + 1))
#define TILEY_PROCF(fy, max)    SK_USHIFT16(tileProcY(fy) * ((max) + 1))
#define EXTRACT_LOW_BITS(v, max) (((v * (max + 1)) >> 12) & 0xF)
#include "SkBitmapProcState_matrix.h"  // Will create GeneralXY_filter_scale.

struct GeneralTileProcs {
    static unsigned X(const SkBitmapProcState& s, SkFixed fx, int max) {
        return SK_USHIFT16(s.fTileProcX(fx) * ((max) + 1));
    }
    static unsigned Y(const SkBitmapProcState& s, SkFixed fy, int max) {
        return SK_USHIFT16(s.fTileProcY(fy) * ((max) + 1));
    }
};

static SkBitmapProcState::MatrixProc GeneralXY_Procs[] = {
    nofilter_scale<GeneralTileProcs, false>,
    GeneralXY_filter_scale,
};

///////////////////////////////////////////////////////////////////////////////
// FixedTileProcs for use with GeneralTileProcs.

static inline U16CPU fixed_clamp(SkFixed x) {
    if (x <   0) { x = 0; }
    if (x >> 16) { x = 0xFFFF; }
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
    switch (m) {
        default: SkASSERT(false);
        case SkShader::kClamp_TileMode:  return fixed_clamp;
        case SkShader::kRepeat_TileMode: return fixed_repeat;
        case SkShader::kMirror_TileMode: return fixed_mirror;
    }
}

///////////////////////////////////////////////////////////////////////////////
// The main entry point to the file, choosing between everything above.

SkBitmapProcState::MatrixProc SkBitmapProcState::chooseMatrixProc() {
    SkASSERT(fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));

    // The arrays are all [ nofilter, filter ].
    int index = fFilterQuality > kNone_SkFilterQuality ? 1 : 0;

    if (SkShader::kClamp_TileMode == fTileModeX &&
        SkShader::kClamp_TileMode == fTileModeY) {
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

    if (SkShader::kRepeat_TileMode == fTileModeX &&
        SkShader::kRepeat_TileMode == fTileModeY) {
    #if defined(SK_ARM_HAS_NEON)
        return RepeatX_RepeatY_Procs_neon[index];
    #else
        return RepeatX_RepeatY_Procs[index];
    #endif
    }

    fTileProcX = choose_tile_proc(fTileModeX);
    fTileProcY = choose_tile_proc(fTileModeY);
    return GeneralXY_Procs[index];
}
