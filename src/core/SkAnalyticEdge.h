/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnalyticEdge_DEFINED
#define SkAnalyticEdge_DEFINED

#include "include/private/SkTo.h"
#include "src/core/SkEdge.h"

#include <utility>

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

    bool setLine(const SkPoint& p0, const SkPoint& p1);
    bool updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by, SkFixed slope);

    // return true if we're NOT done with this edge
    bool update(SkFixed last_y, bool sortY = true);

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

    bool setCubic(const SkPoint pts[4], bool sortY = true);
    bool updateCubic(bool sortY = true);
    inline void keepContinuous() {
        SkASSERT(SkAbs32(fX - SkFixedMul(fDX, fY - SnapY(fCEdge.fCy)) - fCEdge.fCx) < SK_Fixed1);
        fCEdge.fCx = fX;
        fSnappedY = fY;
    }
};

struct SkBezier {
    int fCount; // 2 line, 3 quad, 4 cubic
    SkPoint fP0;
    SkPoint fP1;

    // See if left shift, covert to SkFDot6, and round has the same top and bottom y.
    // If so, the edge will be empty.
    static inline bool IsEmpty(SkScalar y0, SkScalar y1, int shift = 2) {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        return SkScalarRoundToFDot6(y0, shift) == SkScalarRoundToFDot6(y1, shift);
#else
        SkScalar scale = (1 << (shift + 6));
        return SkFDot6Round(int(y0 * scale)) == SkFDot6Round(int(y1 * scale));
#endif
    }
};

struct SkLine : public SkBezier {
    bool set(const SkPoint pts[2]){
        if (IsEmpty(pts[0].fY, pts[1].fY)) {
            return false;
        }
        fCount = 2;
        fP0 = pts[0];
        fP1 = pts[1];
        return true;
    }
};

struct SkQuad : public SkBezier {
    SkPoint fP2;

    bool set(const SkPoint pts[3]){
        if (IsEmpty(pts[0].fY, pts[2].fY)) {
            return false;
        }
        fCount = 3;
        fP0 = pts[0];
        fP1 = pts[1];
        fP2 = pts[2];
        return true;
    }
};

struct SkCubic : public SkBezier {
    SkPoint fP2;
    SkPoint fP3;

    bool set(const SkPoint pts[4]){
        // We do not chop at y extrema for cubics so pts[0], pts[1], pts[2], pts[3] may not be
        // monotonic. Therefore, we have to check the emptiness for all three pairs, instead of just
        // checking IsEmpty(pts[0].fY, pts[3].fY).
        if (IsEmpty(pts[0].fY, pts[1].fY) && IsEmpty(pts[1].fY, pts[2].fY) &&
                IsEmpty(pts[2].fY, pts[3].fY)) {
            return false;
        }
        fCount = 4;
        fP0 = pts[0];
        fP1 = pts[1];
        fP2 = pts[2];
        fP3 = pts[3];
        return true;
    }
};

#endif
