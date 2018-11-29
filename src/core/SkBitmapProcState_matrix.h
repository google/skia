/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMath.h"
#include "SkMathPriv.h"

#define SCALE_FILTER_NAME MAKENAME(_filter_scale)

// declare functions externally to suppress warnings.
void SCALE_FILTER_NAME(const SkBitmapProcState& s,
                       uint32_t xy[], int count, int x, int y);


void SCALE_FILTER_NAME(const SkBitmapProcState& s,
                       uint32_t xy[], int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);

    auto pack = [](SkFixed f, unsigned max, SkFixed one) {
        unsigned i = TILE_PROCF(f, max);
        i = (i << 4) | EXTRACT_LOW_BITS(f, max);
        return (i << 14) | (TILE_PROCF((f + one), max));
    };

    const unsigned maxX = s.fPixmap.width() - 1;
    const SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const SkFixed fy = mapper.fixedY();
        const unsigned maxY = s.fPixmap.height() - 1;
        // compute our two Y values up front
        *xy++ = pack(fy, maxY, s.fFilterOneY);
        // now initialize fx
        fx = mapper.fractionalIntX();
    }

#ifdef CHECK_FOR_DECAL
    // For historical reasons we check both ends are < maxX rather than <= maxX.
    // TODO: try changing this?  See also can_truncate_to_fixed_for_decal().
    if ((unsigned)SkFractionalIntToInt(fx               ) < maxX &&
        (unsigned)SkFractionalIntToInt(fx + dx*(count-1)) < maxX) {
        while (count --> 0) {
            SkFixed fixedFx = SkFractionalIntToFixed(fx);
            SkASSERT((fixedFx >> (16 + 14)) == 0);
            *xy++ = (fixedFx >> 12 << 14) | ((fixedFx >> 16) + 1);
            fx += dx;
        }
        return;
    }
#endif
    while (count --> 0) {
        SkFixed fixedFx = SkFractionalIntToFixed(fx);
        *xy++ = pack(fixedFx, maxX, s.fFilterOneX);
        fx += dx;
    }
}

#undef MAKENAME
#undef TILE_PROCF
#undef EXTRACT_LOW_BITS
#ifdef CHECK_FOR_DECAL
    #undef CHECK_FOR_DECAL
#endif

#undef SCALE_FILTER_NAME
