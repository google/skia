/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnalyticEdge_DEFINED
#define SkAnalyticEdge_DEFINED

#include "SkEdge.h"

struct SkAnalyticEdge {
    // Similar to SkEdge, the conic edges will be converted to quadratic edges
    enum Type {
        kLine_Type,
        kQuad_Type,
        kCubic_Type
    };

    SkAnalyticEdge* fNext;
    SkAnalyticEdge* fPrev;

    // During aaa_walk_edges, if this edge is a left edge,
    // then fRiteE is its corresponding right edge. Otherwise it's nullptr.
    SkAnalyticEdge* fRiteE;

    SkFixed fX;
    SkFixed fDX;
    SkFixed fUpperX;        // The x value when y = fUpperY
    SkFixed fY;             // The current y
    SkFixed fUpperY;        // The upper bound of y (our edge is from y = fUpperY to y = fLowerY)
    SkFixed fLowerY;        // The lower bound of y (our edge is from y = fUpperY to y = fLowerY)
    SkFixed fDY;            // abs(1/fDX); may be SK_MaxS32 when fDX is close to 0.
                            // fDY is only used for blitting trapezoids.

    SkFixed fSavedX;        // For deferred blitting
    SkFixed fSavedY;        // For deferred blitting
    SkFixed fSavedDY;       // For deferred blitting

    int8_t  fCurveCount;    // only used by kQuad(+) and kCubic(-)
    uint8_t fCurveShift;    // appled to all Dx/DDx/DDDx except for fCubicDShift exception
    uint8_t fCubicDShift;   // applied to fCDx and fCDy only in cubic
    int8_t  fWinding;       // 1 or -1

    static const int kDefaultAccuracy = 2; // default accuracy for snapping

    static inline SkFixed SnapY(SkFixed y) {
        const int accuracy = kDefaultAccuracy;
        // This approach is safer than left shift, round, then right shift
        return ((unsigned)y + (SK_Fixed1 >> (accuracy + 1))) >> (16 - accuracy) << (16 - accuracy);
    }

    // Update fX, fY of this edge so fY = y
    inline void goY(SkFixed y) {
        if (y == fY + SK_Fixed1) {
            fX = fX + fDX;
            fY = y;
        } else if (y != fY) {
            // Drop lower digits as our alpha only has 8 bits
            // (fDX and y - fUpperY may be greater than SK_Fixed1)
            fX = fUpperX + SkFixedMul(fDX, y - fUpperY);
            fY = y;
        }
    }

    inline void goY(SkFixed y, int yShift) {
        SkASSERT(yShift >= 0 && yShift <= kDefaultAccuracy);
        SkASSERT(fDX == 0 || y - fY == SK_Fixed1 >> yShift);
        fY = y;
        fX += fDX >> yShift;
    }

    inline void saveXY(SkFixed x, SkFixed y, SkFixed dY) {
        fSavedX = x;
        fSavedY = y;
        fSavedDY = dY;
    }

    inline bool setLine(const SkPoint& p0, const SkPoint& p1);
    inline bool updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by, SkFixed slope);

#ifdef SK_DEBUG
    void dump() const {
        SkDebugf("edge: upperY:%d lowerY:%d y:%g x:%g dx:%g w:%d\n",
                 fUpperY, fLowerY, SkFixedToFloat(fY), SkFixedToFloat(fX),
                 SkFixedToFloat(fDX), fWinding);
    }

    void validate() const {
         SkASSERT(fPrev && fNext);
         SkASSERT(fPrev->fNext == this);
         SkASSERT(fNext->fPrev == this);

         SkASSERT(fUpperY < fLowerY);
         SkASSERT(SkAbs32(fWinding) == 1);
    }
#endif
};

struct SkAnalyticQuadraticEdge : public SkAnalyticEdge {
    SkQuadraticEdge fQEdge;

    // snap y to integer points in the middle of the curve to accelerate AAA path filling
    SkFixed fSnappedX, fSnappedY;

    bool setQuadratic(const SkPoint pts[3]);
    bool updateQuadratic();
    inline void keepContinuous() {
        // We use fX as the starting x to ensure the continuouty.
        // Without it, we may break the sorted edge list.
        SkASSERT(SkAbs32(fX - SkFixedMul(fY - fSnappedY, fDX) - fSnappedX) < SK_Fixed1);
        SkASSERT(SkAbs32(fY - fSnappedY) < SK_Fixed1); // This may differ due to smooth jump
        fSnappedX = fX;
        fSnappedY = fY;
    }
};

struct SkAnalyticCubicEdge : public SkAnalyticEdge {
    SkCubicEdge fCEdge;

    SkFixed fSnappedY; // to make sure that y is increasing with smooth jump and snapping

    bool setCubic(const SkPoint pts[4]);
    bool updateCubic();
    inline void keepContinuous() {
        SkASSERT(SkAbs32(fX - SkFixedMul(fDX, fY - SnapY(fCEdge.fCy)) - fCEdge.fCx) < SK_Fixed1);
        fCEdge.fCx = fX;
        fSnappedY = fY;
    }
};

bool SkAnalyticEdge::setLine(const SkPoint& p0, const SkPoint& p1) {
    fRiteE = nullptr;

    // We must set X/Y using the same way (e.g., times 4, to FDot6, then to Fixed) as Quads/Cubics.
    // Otherwise the order of the edge might be wrong due to precision limit.
    const int accuracy = kDefaultAccuracy;
    const int multiplier = (1 << kDefaultAccuracy);
    SkFixed x0 = SkFDot6ToFixed(SkScalarToFDot6(p0.fX * multiplier)) >> accuracy;
    SkFixed y0 = SnapY(SkFDot6ToFixed(SkScalarToFDot6(p0.fY * multiplier)) >> accuracy);
    SkFixed x1 = SkFDot6ToFixed(SkScalarToFDot6(p1.fX * multiplier)) >> accuracy;
    SkFixed y1 = SnapY(SkFDot6ToFixed(SkScalarToFDot6(p1.fY * multiplier)) >> accuracy);

    int winding = 1;

    if (y0 > y1) {
        SkTSwap(x0, x1);
        SkTSwap(y0, y1);
        winding = -1;
    }

    // are we a zero-height line?
    SkFDot6 dy = SkFixedToFDot6(y1 - y0);
    if (dy == 0) {
        return false;
    }
    SkFDot6 dx = SkFixedToFDot6(x1 - x0);
    SkFixed slope = QuickSkFDot6Div(dx, dy);
    SkFixed absSlope = SkAbs32(slope);

    fX          = x0;
    fDX         = slope;
    fUpperX     = x0;
    fY          = y0;
    fUpperY     = y0;
    fLowerY     = y1;
    fDY         = dx == 0 || slope == 0 ? SK_MaxS32 : absSlope < kInverseTableSize
                                                    ? QuickFDot6Inverse::Lookup(absSlope)
                                                    : SkAbs32(QuickSkFDot6Div(dy, dx));
    fCurveCount = 0;
    fWinding    = SkToS8(winding);
    fCurveShift = 0;

    return true;
}

#endif
