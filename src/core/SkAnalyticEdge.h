/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnalyticEdge_DEFINED
#define SkAnalyticEdge_DEFINED

#include "SkEdge.h"

inline SkFixed SkFixedMul_lowprec(SkFixed a, SkFixed b) {
    SkASSERT(((int64_t)a >> 8) * (b >> 8) <= SK_MaxS32);
    return (a >> 8) * (b >> 8);
}

struct SkAnalyticEdge {
    // Similar to SkEdge, the conic edges will be converted to quadratic edges
    enum Type {
        kLine_Type,
        kQuad_Type,
        kCubic_Type
    };

    SkAnalyticEdge* fNext;
    SkAnalyticEdge* fPrev;

    SkFixed fX;
    SkFixed fDX;
    SkFixed fUpperX;        // The x value when y = fUpperY
    SkFixed fY;             // The current y
    SkFixed fUpperY;        // The upper bound of y (our edge is from y = fUpperY to y = fLowerY)
    SkFixed fLowerY;        // The lower bound of y (our edge is from y = fUpperY to y = fLowerY)
    SkFixed fDY;            // abs(1/fDX); may be SK_MaxS32 when fDX is close to 0.
                            // fDY is only used for blitting trapezoids.

    int8_t  fCurveCount;    // only used by kQuad(+) and kCubic(-)
    uint8_t fCurveShift;    // appled to all Dx/DDx/DDDx except for fCubicDShift exception
    uint8_t fCubicDShift;   // applied to fCDx and fCDy only in cubic
    int8_t  fWinding;       // 1 or -1

    static const int kDefaultAccuracy = 2; // default accuracy for snapping

    static inline SkFixed snapY(SkFixed y, int accuracy = kDefaultAccuracy) {
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
            fX = fUpperX + SkFixedMul_lowprec(fDX, y - fUpperY);
            fY = y;
        }
    }

    inline bool setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip = nullptr);
    inline bool updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by, SkFixed slope);
    void chopLineWithClip(const SkIRect& clip);

    inline bool intersectsClip(const SkIRect& clip) const {
        SkASSERT(SkFixedFloorToInt(fUpperY) < clip.fBottom);
        return SkFixedCeilToInt(fLowerY) > clip.fTop;
    }

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
};

struct SkAnalyticCubicEdge : public SkAnalyticEdge {
    SkCubicEdge fCEdge;

    bool setCubic(const SkPoint pts[4]);
    bool updateCubic();
};

bool SkAnalyticEdge::setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip) {
    // We must set X/Y using the same way (times 4, to FDot6, then to Fixed) as Quads/Cubics.
    // Otherwise the order of the edge might be wrong due to precision limit.
    SkFixed x0 = SkFDot6ToFixed(SkScalarToFDot6(p0.fX * 4)) >> 2;
    SkFixed y0 = snapY(SkFDot6ToFixed(SkScalarToFDot6(p0.fY * 4)) >> 2);
    SkFixed x1 = SkFDot6ToFixed(SkScalarToFDot6(p1.fX * 4)) >> 2;
    SkFixed y1 = snapY(SkFDot6ToFixed(SkScalarToFDot6(p1.fY * 4)) >> 2);

    // are we a zero-height line?
    if (y0 == y1) {
        return false;
    }

    int top = SkFixedFloorToInt(y0);
    int bot = SkFixedCeilToInt(y1);

    // are we completely above or below the clip?
    if (clip && (top >= clip->fBottom || bot <= clip->fTop)) {
        return false;
    }

    int winding = 1;

    if (y0 > y1) {
        SkTSwap(x0, x1);
        SkTSwap(y0, y1);
        winding = -1;
    }

    SkFixed slope = SkFixedDiv(x1 - x0, y1 - y0);

    fX          = x0;
    fDX         = slope;
    fUpperX     = x0;
    fY          = y0;
    fUpperY     = y0;
    fLowerY     = y1;
    fDY         = x1 != x0 ? SkAbs32(SkFixedDiv(y1 - y0, x1 - x0)) : SK_MaxS32;
    fCurveCount = 0;
    fWinding    = SkToS8(winding);
    fCurveShift = 0;

    if (clip) {
        this->chopLineWithClip(*clip);
    }
    return true;
}

#endif
