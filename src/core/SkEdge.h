
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkEdge_DEFINED
#define SkEdge_DEFINED

#include "SkRect.h"

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

    int setLine(const SkPoint& p0, const SkPoint& p1, const SkIRect* clip,
                int shiftUp);
    inline int updateLine(SkFixed ax, SkFixed ay, SkFixed bx, SkFixed by);
    void chopLineWithClip(const SkIRect& clip);

    inline bool intersectsClip(const SkIRect& clip) const {
        SkASSERT(fFirstY < clip.fBottom);
        return fLastY >= clip.fTop;
    }

#ifdef SK_DEBUG
    void dump() const {
    #ifdef SK_CAN_USE_FLOAT
        SkDebugf("edge: firstY:%d lastY:%d x:%g dx:%g w:%d\n", fFirstY, fLastY, SkFixedToFloat(fX), SkFixedToFloat(fDX), fWinding);
    #else
        SkDebugf("edge: firstY:%d lastY:%d x:%x dx:%x w:%d\n", fFirstY, fLastY, fX, fDX, fWinding);
    #endif
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

    int setQuadratic(const SkPoint pts[3], int shiftUp);
    int updateQuadratic();
};

struct SkCubicEdge : public SkEdge {
    SkFixed fCx, fCy;
    SkFixed fCDx, fCDy;
    SkFixed fCDDx, fCDDy;
    SkFixed fCDDDx, fCDDDy;
    SkFixed fCLastX, fCLastY;

    int setCubic(const SkPoint pts[4], const SkIRect* clip, int shiftUp);
    int updateCubic();
};

#endif
