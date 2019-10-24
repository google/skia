/*
 * Copyright 2008 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/private/SkTo.h"
#include "src/core/SkBitmapProcState.h"
#include "src/core/SkUtils.h"

/*
 *  The decal_ functions require that
 *  1. dx > 0
 *  2. [fx, fx+dx, fx+2dx, fx+3dx, ... fx+(count-1)dx] are all <= maxX
 *
 *  In addition, we use SkFractionalInt to keep more fractional precision than
 *  just SkFixed, so we will abort the decal_ call if dx is very small, since
 *  the decal_ function just operates on SkFixed. If that were changed, we could
 *  skip the very_small test here.
 */
static inline bool can_truncate_to_fixed_for_decal(SkFixed fx,
                                                   SkFixed dx,
                                                   int count, unsigned max) {
    SkASSERT(count > 0);

    // if decal_ kept SkFractionalInt precision, this would just be dx <= 0
    // I just made up the 1/256. Just don't want to perceive accumulated error
    // if we truncate frDx and lose its low bits.
    if (dx <= SK_Fixed1 / 256) {
        return false;
    }

    // Note: it seems the test should be (fx <= max && lastFx <= max); but
    // historically it's been a strict inequality check, and changing produces
    // unexpected diffs.  Further investigation is needed.

    // We cast to unsigned so we don't have to check for negative values, which
    // will now appear as very large positive values, and thus fail our test!
    if ((unsigned)SkFixedFloorToInt(fx) >= max) {
        return false;
    }

    // Promote to 64bit (48.16) to avoid overflow.
    const uint64_t lastFx = fx + sk_64_mul(dx, count - 1);

    return SkTFitsIn<int32_t>(lastFx) && (unsigned)SkFixedFloorToInt(SkTo<int32_t>(lastFx)) < max;
}

// When not filtering, we store 32-bit y, 16-bit x, 16-bit x, 16-bit x, ...
// When filtering we write out 32-bit encodings, pairing 14.4 x0 with 14-bit x1.

// The clamp routines may try to fall into one of these unclamped decal fast-paths.
// (Only clamp works in the right coordinate space to check for decal.)
static void decal_nofilter_scale(uint32_t dst[], SkFixed fx, SkFixed dx, int count) {
    // can_truncate_to_fixed_for_decal() checked only that stepping fx+=dx count-1
    // times doesn't overflow fx, so we take unusual care not to step count times.
    for (; count > 2; count -= 2) {
        *dst++ = pack_two_shorts( (fx +  0) >> 16,
                                  (fx + dx) >> 16);
        fx += dx+dx;
    }

    SkASSERT(count <= 2);
    switch (count) {
        case 2: ((uint16_t*)dst)[1] = SkToU16((fx + dx) >> 16);
        case 1: ((uint16_t*)dst)[0] = SkToU16((fx +  0) >> 16);
    }
}

// A generic implementation for unfiltered scale+translate, templated on tiling method.
template <unsigned (*tile)(SkFixed, int), bool tryDecal>
static void nofilter_scale(const SkBitmapProcState& s,
                           uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isScaleTranslate());

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

template <unsigned (*tile)(SkFixed, int)>
static void nofilter_affine(const SkBitmapProcState& s,
                            uint32_t xy[], int count, int x, int y) {
    SkASSERT(!s.fInvMatrix.hasPerspective());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFractionalInt fx = mapper.fractionalIntX(),
                    fy = mapper.fractionalIntY(),
                    dx = s.fInvSxFractionalInt,
                    dy = s.fInvKyFractionalInt;
    int maxX = s.fPixmap.width () - 1,
        maxY = s.fPixmap.height() - 1;

    while (count --> 0) {
        *xy++ = (tile(SkFractionalIntToFixed(fy), maxY) << 16)
              | (tile(SkFractionalIntToFixed(fx), maxX)      );
        fx += dx;
        fy += dy;
    }
}

// Extract the high four fractional bits from fx, the lerp parameter when filtering.
static unsigned extract_low_bits_clamp(SkFixed fx, int /*max*/) {
    // If we're already scaled up to by max like clamp/decal,
    // just grab the high four fractional bits.
    return (fx >> 12) & 0xf;
}
static unsigned extract_low_bits_repeat_mirror(SkFixed fx, int max) {
    // In repeat or mirror fx is in [0,1], so scale up by max first.
    // TODO: remove the +1 here and the -1 at the call sites...
    return extract_low_bits_clamp((fx & 0xffff) * (max+1), max);
}

template <unsigned (*tile)(SkFixed, int), unsigned (*extract_low_bits)(SkFixed, int)>
static uint32_t pack(SkFixed f, unsigned max, SkFixed one) {
    uint32_t packed = tile(f, max);                      // low coordinate in high bits
    packed = (packed <<  4) | extract_low_bits(f, max);  // (lerp weight _is_ coord fractional part)
    packed = (packed << 14) | tile((f + one), max);      // high coordinate in low bits
    return packed;
}

template <unsigned (*tile)(SkFixed, int), unsigned (*extract_low_bits)(SkFixed, int), bool tryDecal>
static void filter_scale(const SkBitmapProcState& s,
                         uint32_t xy[], int count, int x, int y) {
    SkASSERT(s.fInvMatrix.isScaleTranslate());

    const unsigned maxX = s.fPixmap.width() - 1;
    const SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const unsigned maxY = s.fPixmap.height() - 1;
        // compute our two Y values up front
        *xy++ = pack<tile, extract_low_bits>(mapper.fixedY(), maxY, s.fFilterOneY);
        // now initialize fx
        fx = mapper.fractionalIntX();
    }

    // For historical reasons we check both ends are < maxX rather than <= maxX.
    // TODO: try changing this?  See also can_truncate_to_fixed_for_decal().
    if (tryDecal &&
        (unsigned)SkFractionalIntToInt(fx               ) < maxX &&
        (unsigned)SkFractionalIntToInt(fx + dx*(count-1)) < maxX) {
        while (count --> 0) {
            SkFixed fixedFx = SkFractionalIntToFixed(fx);
            SkASSERT((fixedFx >> (16 + 14)) == 0);
            *xy++ = (fixedFx >> 12 << 14) | ((fixedFx >> 16) + 1);
            fx += dx;
        }
        return;
    }

    while (count --> 0) {
        *xy++ = pack<tile, extract_low_bits>(SkFractionalIntToFixed(fx), maxX, s.fFilterOneX);
        fx += dx;
    }
}

template <unsigned (*tile)(SkFixed, int), unsigned (*extract_low_bits)(SkFixed, int)>
static void filter_affine(const SkBitmapProcState& s,
                          uint32_t xy[], int count, int x, int y) {
    SkASSERT(!s.fInvMatrix.hasPerspective());

    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFixed oneX = s.fFilterOneX,
            oneY = s.fFilterOneY;

    SkFractionalInt fx = mapper.fractionalIntX(),
                    fy = mapper.fractionalIntY(),
                    dx = s.fInvSxFractionalInt,
                    dy = s.fInvKyFractionalInt;
    unsigned maxX = s.fPixmap.width () - 1,
             maxY = s.fPixmap.height() - 1;
    while (count --> 0) {
        *xy++ = pack<tile, extract_low_bits>(SkFractionalIntToFixed(fy), maxY, oneY);
        *xy++ = pack<tile, extract_low_bits>(SkFractionalIntToFixed(fx), maxX, oneX);

        fy += dy;
        fx += dx;
    }
}

// Helper to ensure that when we shift down, we do it w/o sign-extension
// so the caller doesn't have to manually mask off the top 16 bits.
static inline unsigned SK_USHIFT16(unsigned x) {
    return x >> 16;
}

static unsigned clamp(SkFixed fx, int max) {
    return SkClampMax(fx >> 16, max);
}
static unsigned repeat(SkFixed fx, int max) {
    SkASSERT(max < 65535);
    return SK_USHIFT16((unsigned)(fx & 0xFFFF) * (max + 1));
}
static unsigned mirror(SkFixed fx, int max) {
    SkASSERT(max < 65535);
    // s is 0xFFFFFFFF if we're on an odd interval, or 0 if an even interval
    SkFixed s = SkLeftShift(fx, 15) >> 31;

    // This should be exactly the same as repeat(fx ^ s, max) from here on.
    return SK_USHIFT16( ((fx ^ s) & 0xFFFF) * (max + 1) );
}

static const SkBitmapProcState::MatrixProc ClampX_ClampY_Procs[] = {
    nofilter_scale <clamp, true>, filter_scale <clamp, extract_low_bits_clamp, true>,
    nofilter_affine<clamp>,       filter_affine<clamp, extract_low_bits_clamp>,
};
static const SkBitmapProcState::MatrixProc RepeatX_RepeatY_Procs[] = {
    nofilter_scale <repeat, false>, filter_scale <repeat, extract_low_bits_repeat_mirror,false>,
    nofilter_affine<repeat>,        filter_affine<repeat, extract_low_bits_repeat_mirror>,
};
static const SkBitmapProcState::MatrixProc MirrorX_MirrorY_Procs[] = {
    nofilter_scale <mirror, false>, filter_scale <mirror, extract_low_bits_repeat_mirror, false>,
    nofilter_affine<mirror>,        filter_affine<mirror, extract_low_bits_repeat_mirror>,
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
    SkASSERT(s.fInvMatrix.isTranslate());

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
    SkASSERT(s.fInvMatrix.isTranslate());

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
    SkASSERT(s.fInvMatrix.isTranslate());

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
    SkASSERT(!fInvMatrix.hasPerspective());
    SkASSERT(fTileModeX == fTileModeY);
    SkASSERT(fTileModeX != SkTileMode::kDecal);

    // Check for our special case translate methods when there is no scale/affine/perspective.
    if (translate_only_matrix && kNone_SkFilterQuality == fFilterQuality) {
        switch (fTileModeX) {
            default: SkASSERT(false);
            case SkTileMode::kClamp:  return  clampx_nofilter_trans;
            case SkTileMode::kRepeat: return repeatx_nofilter_trans;
            case SkTileMode::kMirror: return mirrorx_nofilter_trans;
        }
    }

    // The arrays are all [ nofilter, filter ].
    int index = fFilterQuality > kNone_SkFilterQuality ? 1 : 0;
    if (!fInvMatrix.isScaleTranslate()) {
        index |= 2;
    }

    if (fTileModeX == SkTileMode::kClamp) {
        // clamp gets special version of filterOne, working in non-normalized space (allowing decal)
        fFilterOneX = SK_Fixed1;
        fFilterOneY = SK_Fixed1;
        return ClampX_ClampY_Procs[index];
    }

    // all remaining procs use this form for filterOne, putting them into normalized space.
    fFilterOneX = SK_Fixed1 / fPixmap.width();
    fFilterOneY = SK_Fixed1 / fPixmap.height();

    if (fTileModeX == SkTileMode::kRepeat) {
        return RepeatX_RepeatY_Procs[index];
    }

    return MirrorX_MirrorY_Procs[index];
}
