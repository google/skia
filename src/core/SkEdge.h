/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkEdge_DEFINED
#define SkEdge_DEFINED

#include "SkRect.h"
#include "SkFDot6.h"
#include "SkMath.h"

// This correctly favors the lower-pixel when y0 is on a 1/2 pixel boundary
#define SkEdge_Compute_DY(top, y0)  (SkLeftShift(top, 6) + 32 - (y0))

struct SkEdge {
    enum Type {
        kLine_Type,
        kQuad_Type,
        kCubic_Type
    };

    SkEdge* fNext;
    SkEdge* fPrev;

    SkFixed fX;
    SkFixed fDX;
    int32_t fFirstY;
    int32_t fLastY;
    int8_t fCurveCount;    // only used by kQuad(+) and kCubic(-)
    uint8_t fCurveShift;    // appled to all Dx/DDx/DDDx except for fCubicDShift exception
    uint8_t fCubicDShift;   // applied to fCDx and fCDy only in cubic
    int8_t  fWinding;       // 1 or -1

    int setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip, int shiftUp);
    // call this version if you know you don't have a clip
    inline int setLine(const SkPoint& p0, const SkPoint& p1, int shiftUp);
    inline int updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by);
    void chopLineWithClip(const SkIRect& clip);

    inline bool intersectsClip(const SkIRect& clip) const {
        SkASSERT(fFirstY < clip.fBottom);
        return fLastY >= clip.fTop;
    }

#ifdef SK_DEBUG
    void dump() const {
        SkDebugf("edge: firstY:%d lastY:%d x:%g dx:%g w:%d\n", fFirstY, fLastY, SkFixedToFloat(fX), SkFixedToFloat(fDX), fWinding);
    }

    void validate() const {
        SkASSERT(fPrev && fNext);
        SkASSERT(fPrev->fNext == this);
        SkASSERT(fNext->fPrev == this);

        SkASSERT(fFirstY <= fLastY);
        SkASSERT(SkAbs32(fWinding) == 1);
    }
#endif
};

struct SkQuadraticEdge : public SkEdge {
    SkFixed fQx, fQy;
    SkFixed fQDx, fQDy;
    SkFixed fQDDx, fQDDy;
    SkFixed fQLastX, fQLastY;

    bool setQuadraticWithoutUpdate(const SkPoint pts[3], int shiftUp);
    int setQuadratic(const SkPoint pts[3], int shiftUp);
    int updateQuadratic();
};

struct SkCubicEdge : public SkEdge {
    SkFixed fCx, fCy;
    SkFixed fCDx, fCDy;
    SkFixed fCDDx, fCDDy;
    SkFixed fCDDDx, fCDDDy;
    SkFixed fCLastX, fCLastY;

    bool setCubicWithoutUpdate(const SkPoint pts[4], int shiftUp);
    int setCubic(const SkPoint pts[4], int shiftUp);
    int updateCubic();
};

int SkEdge::setLine(const SkPoint& p0, const SkPoint& p1, int shift) {
    SkFDot6 x0, y0, x1, y1;

    {
#ifdef SK_RASTERIZE_EVEN_ROUNDING
        x0 = SkScalarRoundToFDot6(p0.fX, shift);
        y0 = SkScalarRoundToFDot6(p0.fY, shift);
        x1 = SkScalarRoundToFDot6(p1.fX, shift);
        y1 = SkScalarRoundToFDot6(p1.fY, shift);
#else
        float scale = float(1 << (shift + 6));
        x0 = int(p0.fX * scale);
        y0 = int(p0.fY * scale);
        x1 = int(p1.fX * scale);
        y1 = int(p1.fY * scale);
#endif
    }

    int winding = 1;

    if (y0 > y1) {
        SkTSwap(x0, x1);
        SkTSwap(y0, y1);
        winding = -1;
    }

    int top = SkFDot6Round(y0);
    int bot = SkFDot6Round(y1);

    // are we a zero-height line?
    if (top == bot) {
        return 0;
    }

    SkFixed slope = SkFDot6Div(x1 - x0, y1 - y0);
    const SkFDot6 dy  = SkEdge_Compute_DY(top, y0);

    fX          = SkFDot6ToFixed(x0 + SkFixedMul(slope, dy));   // + SK_Fixed1/2
    fDX         = slope;
    fFirstY     = top;
    fLastY      = bot - 1;
    fCurveCount = 0;
    fWinding    = SkToS8(winding);
    fCurveShift = 0;
    return 1;
}

#endif
