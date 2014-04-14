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

    const unsigned maxX = s.fBitmap->width() - 1;
    SkFractionalInt fx;
    {
        SkPoint pt;
        s.fInvProc(s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                                  SkIntToScalar(y) + SK_ScalarHalf, &pt);
        fx = SkScalarToFractionalInt(pt.fY);
        const unsigned maxY = s.fBitmap->height() - 1;
        *xy++ = TileProc::Y(s, SkFractionalIntToFixed(fx), maxY);
        fx = SkScalarToFractionalInt(pt.fX);
    }

    if (0 == maxX) {
        // all of the following X values must be 0
        memset(xy, 0, count * sizeof(uint16_t));
        return;
    }

    const SkFractionalInt dx = s.fInvSxFractionalInt;

    if (tryDecal && can_truncate_to_fixed_for_decal(fx, dx, count, maxX)) {
        decal_nofilter_scale(xy, SkFractionalIntToFixed(fx),
                             SkFractionalIntToFixed(dx), count);
    } else {
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

    SkPoint srcPt;
    s.fInvProc(s.fInvMatrix,
               SkIntToScalar(x) + SK_ScalarHalf,
               SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

    SkFractionalInt fx = SkScalarToFractionalInt(srcPt.fX);
    SkFractionalInt fy = SkScalarToFractionalInt(srcPt.fY);
    SkFractionalInt dx = s.fInvSxFractionalInt;
    SkFractionalInt dy = s.fInvKyFractionalInt;
    int maxX = s.fBitmap->width() - 1;
    int maxY = s.fBitmap->height() - 1;

    for (int i = count; i > 0; --i) {
        *xy++ = (TileProc::Y(s, SkFractionalIntToFixed(fy), maxY) << 16) |
                 TileProc::X(s, SkFractionalIntToFixed(fx), maxX);
        fx += dx; fy += dy;
    }
}

template <typename TileProc>
void NoFilterProc_Persp(const SkBitmapProcState& s, uint32_t* SK_RESTRICT xy,
                        int count, int x, int y) {
    SkASSERT(s.fInvType & SkMatrix::kPerspective_Mask);

    int maxX = s.fBitmap->width() - 1;
    int maxY = s.fBitmap->height() - 1;

    SkPerspIter   iter(s.fInvMatrix,
                       SkIntToScalar(x) + SK_ScalarHalf,
                       SkIntToScalar(y) + SK_ScalarHalf, count);

    while ((count = iter.next()) != 0) {
        const SkFixed* SK_RESTRICT srcXY = iter.getXY();
        while (--count >= 0) {
            *xy++ = (TileProc::Y(s, srcXY[1], maxY) << 16) |
                     TileProc::X(s, srcXY[0], maxX);
            srcXY += 2;
        }
    }
}

#endif
