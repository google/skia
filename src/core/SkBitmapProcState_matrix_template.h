/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_MatrixTemplates_DEFINED
#define SkBitmapProcState_MatrixTemplates_DEFINED

#include "SkMath.h"
#include "SkMathPriv.h"

template <typename TileProc, bool tryDecal>
void NoFilterProc_Scale(const SkBitmapProcState& s, uint32_t xy[],
                        int count, int x, int y) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask)) == 0);

    // we store y, x, x, x, x, x

    const unsigned maxX = s.fPixmap.width() - 1;
    SkFractionalInt fx;
    {
        const SkBitmapProcStateAutoMapper mapper(s, x, y);
        const unsigned maxY = s.fPixmap.height() - 1;
        *xy++ = TileProc::Y(s, mapper.fixedY(), maxY);
        fx = mapper.fractionalIntX();
    }

    if (0 == maxX) {
        // all of the following X values must be 0
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

    int i;
    for (i = (count >> 2); i > 0; --i) {
        unsigned a, b;
        a = TileProc::X(s, SkFractionalIntToFixed(fx), maxX); fx += dx;
        b = TileProc::X(s, SkFractionalIntToFixed(fx), maxX); fx += dx;
#ifdef SK_CPU_BENDIAN
        *xy++ = (a << 16) | b;
#else
        *xy++ = (b << 16) | a;
#endif
        a = TileProc::X(s, SkFractionalIntToFixed(fx), maxX); fx += dx;
        b = TileProc::X(s, SkFractionalIntToFixed(fx), maxX); fx += dx;
#ifdef SK_CPU_BENDIAN
        *xy++ = (a << 16) | b;
#else
        *xy++ = (b << 16) | a;
#endif
    }
    uint16_t* xx = (uint16_t*)xy;
    for (i = (count & 3); i > 0; --i) {
        *xx++ = TileProc::X(s, SkFractionalIntToFixed(fx), maxX); fx += dx;
    }
}

// note: we could special-case on a matrix which is skewed in X but not Y.
// this would require a more general setup thatn SCALE does, but could use
// SCALE's inner loop that only looks at dx

template <typename TileProc>
void NoFilterProc_Affine(const SkBitmapProcState& s, uint32_t xy[],
                         int count, int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kAffine_Mask);
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask |
                             SkMatrix::kScale_Mask |
                             SkMatrix::kAffine_Mask)) == 0);

    const SkBitmapProcStateAutoMapper mapper(s, x, y);

    SkFractionalInt fx = mapper.fractionalIntX();
    SkFractionalInt fy = mapper.fractionalIntY();
    SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt dy = s.fInvKyFractionalInt;
    int maxX = s.fPixmap.width() - 1;
    int maxY = s.fPixmap.height() - 1;

    for (int i = count; i > 0; --i) {
        *xy++ = (TileProc::Y(s, SkFractionalIntToFixed(fy), maxY) << 16) |
                 TileProc::X(s, SkFractionalIntToFixed(fx), maxX);
        fx += dx; fy += dy;
    }
}

#endif
