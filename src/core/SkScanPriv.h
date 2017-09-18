/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkScanPriv_DEFINED
#define SkScanPriv_DEFINED

#include "SkPath.h"
#include "SkScan.h"
#include "SkBlitter.h"

class SkScanClipper {
public:
    SkScanClipper(SkBlitter* blitter, const SkRegion* clip, const SkIRect& bounds,
                  bool skipRejectTest = false);

    SkBlitter*      getBlitter() const { return fBlitter; }
    const SkIRect*  getClipRect() const { return fClipRect; }

private:
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
#ifdef SK_DEBUG
    SkRectClipCheckBlitter fRectClipCheckBlitter;
#endif
    SkBlitter*          fBlitter;
    const SkIRect*      fClipRect;
};

void sk_fill_path(const SkPath& path, const SkIRect& clipRect,
                  SkBlitter* blitter, int start_y, int stop_y, int shiftEdgesUp,
                  bool pathContainedInClip);

// blit the rects above and below avoid, clipped to clip
void sk_blit_above(SkBlitter*, const SkIRect& avoid, const SkRegion& clip);
void sk_blit_below(SkBlitter*, const SkIRect& avoid, const SkRegion& clip);

template<class EdgeType>
static inline void remove_edge(EdgeType* edge) {
    edge->fPrev->fNext = edge->fNext;
    edge->fNext->fPrev = edge->fPrev;
}

template<class EdgeType>
static inline void insert_edge_after(EdgeType* edge, EdgeType* afterMe) {
    edge->fPrev = afterMe;
    edge->fNext = afterMe->fNext;
    afterMe->fNext->fPrev = edge;
    afterMe->fNext = edge;
}

template<class EdgeType>
static void backward_insert_edge_based_on_x(EdgeType* edge) {
    SkFixed x = edge->fX;
    EdgeType* prev = edge->fPrev;
    while (prev->fPrev && prev->fX > x) {
        prev = prev->fPrev;
    }
    if (prev->fNext != edge) {
        remove_edge(edge);
        insert_edge_after(edge, prev);
    }
}

// Start from the right side, searching backwards for the point to begin the new edge list
// insertion, marching forwards from here. The implementation could have started from the left
// of the prior insertion, and search to the right, or with some additional caching, binary
// search the starting point. More work could be done to determine optimal new edge insertion.
template<class EdgeType>
static EdgeType* backward_insert_start(EdgeType* prev, SkFixed x) {
    while (prev->fPrev && prev->fX > x) {
        prev = prev->fPrev;
    }
    return prev;
}

static bool fitsInsideLimit(const SkRect& r, SkScalar max) {
    const SkScalar min = -max;
    return  r.fLeft > min && r.fTop > min &&
            r.fRight < max && r.fBottom < max;
}

static int overflows_short_shift(int value, int shift) {
    const int s = 16 + shift;
    return (SkLeftShift(value, s) >> s) - value;
}

/**
  Would any of the coordinates of this rectangle not fit in a short,
  when left-shifted by shift?
*/
static int rect_overflows_short_shift(SkIRect rect, int shift) {
    SkASSERT(!overflows_short_shift(8191, shift));
    SkASSERT(overflows_short_shift(8192, shift));
    SkASSERT(!overflows_short_shift(32767, 0));
    SkASSERT(overflows_short_shift(32768, 0));

    // Since we expect these to succeed, we bit-or together
    // for a tiny extra bit of speed.
    return overflows_short_shift(rect.fLeft, shift) |
           overflows_short_shift(rect.fRight, shift) |
           overflows_short_shift(rect.fTop, shift) |
           overflows_short_shift(rect.fBottom, shift);
}

static bool safeRoundOut(const SkRect& src, SkIRect* dst, int32_t maxInt) {
    const SkScalar maxScalar = SkIntToScalar(maxInt);

    if (fitsInsideLimit(src, maxScalar)) {
        src.roundOut(dst);
        return true;
    }
    return false;
}

using FillPathFunc = std::function<void(const SkPath& path, SkBlitter* blitter, bool isInverse,
        const SkIRect& ir, const SkRegion* clipRgn, const SkIRect* clipRect, bool forceRLE)>;

static inline void do_fill_path(const SkPath& path, const SkRegion& origClip, SkBlitter* blitter,
        bool forceRLE, const int SHIFT, FillPathFunc fillPathFunc) {
    if (origClip.isEmpty()) {
        return;
    }

    const bool isInverse = path.isInverseFillType();
    SkIRect ir;

    if (!safeRoundOut(path.getBounds(), &ir, SK_MaxS32 >> SHIFT)) {
        // Bounds can't fit in SkIRect; we'll return without drawing
        return;
    }
    if (ir.isEmpty()) {
        if (isInverse) {
            blitter->blitRegion(origClip);
        }
        return;
    }

    // If the intersection of the path bounds and the clip bounds
    // will overflow 32767 when << by SHIFT, we can't supersample,
    // so draw without antialiasing.
    SkIRect clippedIR;
    if (isInverse) {
       // If the path is an inverse fill, it's going to fill the entire
       // clip, and we care whether the entire clip exceeds our limits.
       clippedIR = origClip.getBounds();
    } else {
       if (!clippedIR.intersect(ir, origClip.getBounds())) {
           return;
       }
    }
    if (rect_overflows_short_shift(clippedIR, SHIFT)) {
        SkScan::FillPath(path, origClip, blitter);
        return;
    }

    // Our antialiasing can't handle a clip larger than 32767, so we restrict
    // the clip to that limit here. (the runs[] uses int16_t for its index).
    //
    // A more general solution (one that could also eliminate the need to
    // disable aa based on ir bounds (see overflows_short_shift) would be
    // to tile the clip/target...
    SkRegion tmpClipStorage;
    const SkRegion* clipRgn = &origClip;
    {
        static const int32_t kMaxClipCoord = 32767;
        const SkIRect& bounds = origClip.getBounds();
        if (bounds.fRight > kMaxClipCoord || bounds.fBottom > kMaxClipCoord) {
            SkIRect limit = { 0, 0, kMaxClipCoord, kMaxClipCoord };
            tmpClipStorage.op(origClip, limit, SkRegion::kIntersect_Op);
            clipRgn = &tmpClipStorage;
        }
    }
    // for here down, use clipRgn, not origClip

    SkScanClipper   clipper(blitter, clipRgn, ir);
    const SkIRect*  clipRect = clipper.getClipRect();

    if (clipper.getBlitter() == nullptr) { // clipped out
        if (isInverse) {
            blitter->blitRegion(*clipRgn);
        }
        return;
    }

    SkASSERT(clipper.getClipRect() == nullptr ||
            *clipper.getClipRect() == clipRgn->getBounds());

    // now use the (possibly wrapped) blitter
    blitter = clipper.getBlitter();

    if (isInverse) {
        sk_blit_above(blitter, ir, *clipRgn);
    }

    SkASSERT(SkIntToScalar(ir.fTop) <= path.getBounds().fTop);

    fillPathFunc(path, blitter, isInverse, ir, clipRgn, clipRect, forceRLE);

    if (isInverse) {
        sk_blit_below(blitter, ir, *clipRgn);
    }
}

#endif
