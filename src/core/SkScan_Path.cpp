/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkSafe32.h"
#include "src/base/SkTSort.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkEdge.h"
#include "src/core/SkEdgeBuilder.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkScan.h"
#include "src/core/SkScanPriv.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

struct SkMask;

#define kEDGE_HEAD_Y    SK_MinS32
#define kEDGE_TAIL_Y    SK_MaxS32

#ifdef SK_DEBUG
    static void validate_sort(const SkEdge* edge) {
        int y = kEDGE_HEAD_Y;

        while (edge->fFirstY != SK_MaxS32) {
            edge->validate();
            SkASSERT(y <= edge->fFirstY);

            y = edge->fFirstY;
            edge = edge->fNext;
        }
    }
#else
    #define validate_sort(edge)
#endif

static void insert_new_edges(SkEdge* newEdge, int curr_y) {
    if (newEdge->fFirstY != curr_y) {
        return;
    }
    SkEdge* prev = newEdge->fPrev;
    if (prev->fX <= newEdge->fX) {
        return;
    }
    // find first x pos to insert
    SkEdge* start = backward_insert_start(prev, newEdge->fX);
    // insert the lot, fixing up the links as we go
    do {
        SkEdge* next = newEdge->fNext;
        do {
            if (start->fNext == newEdge) {
                goto nextEdge;
            }
            SkEdge* after = start->fNext;
            if (after->fX >= newEdge->fX) {
                break;
            }
            start = after;
        } while (true);
        remove_edge(newEdge);
        insert_edge_after(newEdge, start);
nextEdge:
        start = newEdge;
        newEdge = next;
    } while (newEdge->fFirstY == curr_y);
}

#ifdef SK_DEBUG
static void validate_edges_for_y(const SkEdge* edge, int curr_y) {
    while (edge->fFirstY <= curr_y) {
        SkASSERT(edge->fPrev && edge->fNext);
        SkASSERT(edge->fPrev->fNext == edge);
        SkASSERT(edge->fNext->fPrev == edge);
        SkASSERT(edge->fFirstY <= edge->fLastY);

        SkASSERT(edge->fPrev->fX <= edge->fX);
        edge = edge->fNext;
    }
}
#else
    #define validate_edges_for_y(edge, curr_y)
#endif

#if defined _WIN32  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

typedef void (*PrePostProc)(SkBlitter* blitter, int y, bool isStartOfScanline);
#define PREPOST_START   true
#define PREPOST_END     false

static void walk_edges(SkEdge* prevHead, SkPathFillType fillType,
                       SkBlitter* blitter, int start_y, int stop_y,
                       PrePostProc proc, int rightClip) {
    validate_sort(prevHead->fNext);

    int curr_y = start_y;
    int windingMask = SkPathFillType_IsEvenOdd(fillType) ? 1 : -1;

    for (;;) {
        int     w = 0;
        int     left SK_INIT_TO_AVOID_WARNING;
        SkEdge* currE = prevHead->fNext;
        SkFixed prevX = prevHead->fX;

        validate_edges_for_y(currE, curr_y);

        if (proc) {
            proc(blitter, curr_y, PREPOST_START);    // pre-proc
        }

        while (currE->fFirstY <= curr_y) {
            SkASSERT(currE->fLastY >= curr_y);

            int x = SkFixedRoundToInt(currE->fX);

            if ((w & windingMask) == 0) { // we're starting interval
                left = x;
            }

            w += static_cast<int>(currE->fWinding);

            if ((w & windingMask) == 0) { // we finished an interval
                int width = x - left;
                SkASSERT(width >= 0);
                if (width > 0) {
                    blitter->blitH(left, curr_y, width);
                }
            }

            SkEdge* next = currE->fNext;
            SkFixed newX;

            if (currE->fLastY == curr_y) {    // are we done with this edge?
                if (currE->hasNextSegment()) {
                    if (currE->nextSegment()) {
                        SkASSERT(currE->fFirstY == curr_y + 1);

                        newX = currE->fX;
                        goto NEXT_X;
                    }
                }
                remove_edge(currE);
            } else {
                SkASSERT(currE->fLastY > curr_y);
                newX = currE->fX + currE->fDxDy;
                currE->fX = newX;
            NEXT_X:
                if (newX < prevX) { // ripple currE backwards until it is x-sorted
                    backward_insert_edge_based_on_x(currE);
                } else {
                    prevX = newX;
                }
            }
            currE = next;
            SkASSERT(currE);
        }

        if ((w & windingMask) != 0) { // was our right-edge culled away?
            int width = rightClip - left;
            if (width > 0) {
                blitter->blitH(left, curr_y, width);
            }
        }

        if (proc) {
            proc(blitter, curr_y, PREPOST_END);    // post-proc
        }

        curr_y += 1;
        if (curr_y >= stop_y) {
            break;
        }
        // now currE points to the first edge with a Yint larger than curr_y
        insert_new_edges(currE, curr_y);
    }
}

// return true if we're NOT done with this edge
static bool update_edge(SkEdge* edge, int last_y) {
    SkASSERT(edge->fLastY >= last_y);
    if (last_y != edge->fLastY) {
        return true;
    }
    if (!edge->hasNextSegment()) {
        return false;
    }
    if (edge->nextSegment()) {
        SkASSERT(edge->fFirstY == last_y + 1);
        return true;
    }
    return false;
}

// Unexpected conditions for which we need to return
#define ASSERT_RETURN(cond)                    \
    do {                                       \
        if (!(cond)) {                         \
            SkDEBUGFAILF("assert(%s)", #cond); \
            return;                            \
        }                                      \
    } while (0)

// Needs Y to only change once (looser than convex in X)
static void walk_simple_edges(SkEdge* prevHead, SkBlitter* blitter, int start_y, int stop_y) {
    validate_sort(prevHead->fNext);

    SkEdge* leftE = prevHead->fNext;
    SkEdge* riteE = leftE->fNext;
    SkEdge* currE = riteE->fNext;

    // our edge choppers for curves can result in the initial edges
    // not lining up, so we take the max.
    int local_top = std::max(leftE->fFirstY, riteE->fFirstY);
    ASSERT_RETURN(local_top >= start_y);

    while (local_top < stop_y) {
        SkASSERT(leftE->fFirstY <= stop_y);
        SkASSERT(riteE->fFirstY <= stop_y);

        int local_bot = std::min(leftE->fLastY, riteE->fLastY);
        local_bot = std::min(local_bot, stop_y - 1);
        ASSERT_RETURN(local_top <= local_bot);

        SkFixed left = leftE->fX;
        SkFixed dLeft = leftE->fDxDy;
        SkFixed rite = riteE->fX;
        SkFixed dRite = riteE->fDxDy;
        int count = local_bot - local_top;
        ASSERT_RETURN(count >= 0);

        if (dLeft == 0 && dRite == 0) {
            int L = SkFixedRoundToInt(left);
            int R = SkFixedRoundToInt(rite);
            if (L > R) {
                std::swap(L, R);
            }
            if (L < R) {
                count += 1;
                blitter->blitRect(L, local_top, R - L, count);
            }
            local_top = local_bot + 1;
        } else {
            do {
                int L = SkFixedRoundToInt(left);
                int R = SkFixedRoundToInt(rite);
                if (L > R) {
                    std::swap(L, R);
                }
                if (L < R) {
                    blitter->blitH(L, local_top, R - L);
                }
                // Either/both of these might overflow, since we perform this step even if
                // (later) we determine that we are done with the edge, and so the computed
                // left or rite edge will not be used (see update_edge). Use this helper to
                // silence UBSAN when we perform the add.
                left = Sk32_can_overflow_add(left, dLeft);
                rite = Sk32_can_overflow_add(rite, dRite);
                local_top += 1;
            } while (--count >= 0);
        }

        leftE->fX = left;
        riteE->fX = rite;

        if (!update_edge(leftE, local_bot)) {
            if (currE->fFirstY >= stop_y) {
                return; // we're done
            }
            leftE = currE;
            currE = currE->fNext;
            ASSERT_RETURN(leftE->fFirstY == local_top);
        }
        if (!update_edge(riteE, local_bot)) {
            if (currE->fFirstY >= stop_y) {
                return; // we're done
            }
            riteE = currE;
            currE = currE->fNext;
            ASSERT_RETURN(riteE->fFirstY == local_top);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

// this overrides blitH, and will call its proxy blitter with the inverse
// of the spans it is given (clipped to the left/right of the cliprect)
//
// used to implement inverse filltypes on paths
//
class InverseBlitter : public SkBlitter {
public:
    void setBlitter(SkBlitter* blitter, const SkIRect& clip) {
        fBlitter = blitter;
        fFirstX = clip.fLeft;
        fLastX = clip.fRight;
    }
    void prepost(int y, bool isStart) {
        if (isStart) {
            fPrevX = fFirstX;
        } else {
            int invWidth = fLastX - fPrevX;
            if (invWidth > 0) {
                fBlitter->blitH(fPrevX, y, invWidth);
            }
        }
    }

    // overrides
    void blitH(int x, int y, int width) override {
        int invWidth = x - fPrevX;
        if (invWidth > 0) {
            fBlitter->blitH(fPrevX, y, invWidth);
        }
        fPrevX = x + width;
    }

    // we do not expect to get called with these entrypoints
    void blitAntiH(int, int, const SkAlpha[], const int16_t runs[]) override {
        SkDEBUGFAIL("blitAntiH unexpected");
    }
    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkDEBUGFAIL("blitV unexpected");
    }
    void blitRect(int x, int y, int width, int height) override {
        SkDEBUGFAIL("blitRect unexpected");
    }
    void blitMask(const SkMask&, const SkIRect& clip) override {
        SkDEBUGFAIL("blitMask unexpected");
    }

private:
    SkBlitter*  fBlitter;
    int         fFirstX, fLastX, fPrevX;
};

static void PrePostInverseBlitterProc(SkBlitter* blitter, int y, bool isStart) {
    ((InverseBlitter*)blitter)->prepost(y, isStart);
}

///////////////////////////////////////////////////////////////////////////////

#if defined _WIN32
#pragma warning ( pop )
#endif

static bool compare_edges(const SkEdge* a, const SkEdge* b) {
    if (a->fFirstY != b->fFirstY) {
        return a->fFirstY < b->fFirstY;
    }

    return a->fX < b->fX;
}

static SkEdge* sort_edges(SkEdge* list[], int count, SkEdge** last) {
    SkTQSort(list, list + count, compare_edges);

    // now make the edges linked in sorted order
    for (int i = 1; i < count; i++) {
        list[i - 1]->fNext = list[i];
        list[i]->fPrev = list[i - 1];
    }

    *last = list[count - 1];
    return list[0];
}

static void sk_fill_path(const SkPath& path, const SkIRect& clipRect, SkBlitter* blitter,
                         int start_y, int stop_y, bool pathContainedInClip) {
    SkASSERT(blitter);

    SkBasicEdgeBuilder builder;
    int count = builder.buildEdges(path, pathContainedInClip ? nullptr : &clipRect);
    SkEdge** list = builder.edgeList();

    if (0 == count) {
        if (path.isInverseFillType()) {
            /*
             *  Since we are in inverse-fill, our caller has already drawn above
             *  our top (start_y) and will draw below our bottom (stop_y). Thus
             *  we need to restrict our drawing to the intersection of the clip
             *  and those two limits.
             */
            SkIRect rect = clipRect;
            if (rect.fTop < start_y) {
                rect.fTop = start_y;
            }
            if (rect.fBottom > stop_y) {
                rect.fBottom = stop_y;
            }
            if (!rect.isEmpty()) {
                blitter->blitRect(rect.fLeft,
                                  rect.fTop,
                                  rect.width(),
                                  rect.height());
            }
        }
        return;
    }

    SkEdge headEdge, tailEdge, *last;
    // this returns the first and last edge after they're sorted into a dlink list
    SkEdge* edge = sort_edges(list, count, &last);

    headEdge.fPrev = nullptr;
    headEdge.fNext = edge;
    headEdge.fFirstY = kEDGE_HEAD_Y;
    headEdge.fX = SK_MinS32;
    edge->fPrev = &headEdge;

    tailEdge.fPrev = last;
    tailEdge.fNext = nullptr;
    tailEdge.fFirstY = kEDGE_TAIL_Y;
    last->fNext = &tailEdge;

    // now edge is the head of the sorted linklist
    if (!pathContainedInClip && start_y < clipRect.fTop) {
        start_y = clipRect.fTop;
    }
    if (!pathContainedInClip && stop_y > clipRect.fBottom) {
        stop_y = clipRect.fBottom;
    }

    InverseBlitter  ib;
    PrePostProc     proc = nullptr;

    if (path.isInverseFillType()) {
        ib.setBlitter(blitter, clipRect);
        blitter = &ib;
        proc = PrePostInverseBlitterProc;
    }

    // count >= 2 is required as the convex walker does not handle missing right edges
    if (path.isConvex() && (nullptr == proc) && count >= 2) {
        walk_simple_edges(&headEdge, blitter, start_y, stop_y);
    } else {
        walk_edges(&headEdge, path.getFillType(), blitter, start_y, stop_y, proc,
                   clipRect.right());
    }
}

void sk_blit_above(SkBlitter* blitter, const SkIRect& ir, const SkRegion& clip) {
    const SkIRect& cr = clip.getBounds();
    SkIRect tmp;

    tmp.fLeft = cr.fLeft;
    tmp.fRight = cr.fRight;
    tmp.fTop = cr.fTop;
    tmp.fBottom = ir.fTop;
    if (!tmp.isEmpty()) {
        blitter->blitRectRegion(tmp, clip);
    }
}

void sk_blit_below(SkBlitter* blitter, const SkIRect& ir, const SkRegion& clip) {
    const SkIRect& cr = clip.getBounds();
    SkIRect tmp;

    tmp.fLeft = cr.fLeft;
    tmp.fRight = cr.fRight;
    tmp.fTop = ir.fBottom;
    tmp.fBottom = cr.fBottom;
    if (!tmp.isEmpty()) {
        blitter->blitRectRegion(tmp, clip);
    }
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  If the caller is drawing an inverse-fill path, then it pass true for
 *  skipRejectTest, so we don't abort drawing just because the src bounds (ir)
 *  is outside of the clip.
 */
SkScanClipper::SkScanClipper(SkBlitter* blitter, const SkRegion* clip,
                             const SkIRect& ir, bool skipRejectTest, bool irPreClipped) {
    fBlitter = nullptr;     // null means blit nothing
    fClipRect = nullptr;

    if (clip) {
        fClipRect = &clip->getBounds();
        if (!skipRejectTest && !SkIRect::Intersects(*fClipRect, ir)) { // completely clipped out
            return;
        }

        if (clip->isRect()) {
            if (!irPreClipped && fClipRect->contains(ir)) {
#ifdef SK_DEBUG
                fRectClipCheckBlitter.init(blitter, *fClipRect);
                blitter = &fRectClipCheckBlitter;
#endif
                fClipRect = nullptr;
            } else {
                // only need a wrapper blitter if we're horizontally clipped
                if (irPreClipped ||
                    fClipRect->fLeft > ir.fLeft || fClipRect->fRight < ir.fRight) {
                    fRectBlitter.init(blitter, *fClipRect);
                    blitter = &fRectBlitter;
                } else {
#ifdef SK_DEBUG
                    fRectClipCheckBlitter.init(blitter, *fClipRect);
                    blitter = &fRectClipCheckBlitter;
#endif
                }
            }
        } else {
            fRgnBlitter.init(blitter, clip);
            blitter = &fRgnBlitter;
        }
    }
    fBlitter = blitter;
}

///////////////////////////////////////////////////////////////////////////////

static bool clip_to_limit(const SkRegion& orig, SkRegion* reduced) {
    // need to limit coordinates such that the width/height of our rect can be represented
    // in SkFixed (16.16). See skbug.com/40039252
    const int32_t limit = 32767 >> 1;

    SkIRect limitR;
    limitR.setLTRB(-limit, -limit, limit, limit);
    if (limitR.contains(orig.getBounds())) {
        return false;
    }
    reduced->op(orig, limitR, SkRegion::kIntersect_Op);
    return true;
}

// Bias used for conservative rounding of float rects to int rects, to nudge the irects a little
// larger, so we don't "think" a path's bounds are inside a clip, when (due to numeric drift in
// the scan-converter) we might walk beyond the predicted limits.
//
// This value has been determined trial and error: pick the smallest value (after the 0.5) that
// fixes any problematic cases (e.g. crbug.com/844457)
// NOTE: cubics appear to be the main reason for needing this slop. If we could (perhaps) have a
// more accurate walker for cubics, we may be able to reduce this fudge factor.
static const double kConservativeRoundBias = 0.5 + 1.5 / SK_FDot6One;

/**
 *  Round the value down. This is used to round the top and left of a rectangle,
 *  and corresponds to the way the scan converter treats the top and left edges.
 *  It has a slight bias to make the "rounded" int smaller than a normal round, to create a more
 *  conservative int-bounds (larger) from a float rect.
 */
static inline int round_down_to_int(SkScalar x) {
    double xx = x;
    xx -= kConservativeRoundBias;
    return sk_double_saturate2int(ceil(xx));
}

/**
 *  Round the value up. This is used to round the right and bottom of a rectangle.
 *  It has a slight bias to make the "rounded" int smaller than a normal round, to create a more
 *  conservative int-bounds (larger) from a float rect.
  */
static inline int round_up_to_int(SkScalar x) {
    double xx = x;
    xx += kConservativeRoundBias;
    return sk_double_saturate2int(floor(xx));
}

/*
 *  Conservative rounding function, which effectively nudges the int-rect to be slightly larger
 *  than SkRect::round() might have produced. This is a safety-net for the scan-converter, which
 *  inspects the returned int-rect, and may disable clipping (for speed) if it thinks all of the
 *  edges will fit inside the clip's bounds. The scan-converter introduces slight numeric errors
 *  due to accumulated += of the slope, so this function is used to return a conservatively large
 *  int-bounds, and thus we will only disable clipping if we're sure the edges will stay in-bounds.
  */
static SkIRect conservative_round_to_int(const SkRect& src) {
    return {
        round_down_to_int(src.fLeft),
        round_down_to_int(src.fTop),
        round_up_to_int(src.fRight),
        round_up_to_int(src.fBottom),
    };
}

void SkScan::FillPath(const SkPath& path, const SkRegion& origClip,
                      SkBlitter* blitter) {
    if (origClip.isEmpty()) {
        return;
    }

    // Our edges are fixed-point, and don't like the bounds of the clip to
    // exceed that. Here we trim the clip just so we don't overflow later on
    const SkRegion* clipPtr = &origClip;
    SkRegion finiteClip;
    if (clip_to_limit(origClip, &finiteClip)) {
        if (finiteClip.isEmpty()) {
            return;
        }
        clipPtr = &finiteClip;
    }
    // don't reference "origClip" any more, just use clipPtr


    SkRect bounds = path.getBounds();
    bool irPreClipped = false;
    if (!SkRectPriv::MakeLargeS32().contains(bounds)) {
        if (!bounds.intersect(SkRectPriv::MakeLargeS32())) {
            bounds.setEmpty();
        }
        irPreClipped = true;
    }

    SkIRect ir = conservative_round_to_int(bounds);
    if (ir.isEmpty()) {
        if (path.isInverseFillType()) {
            blitter->blitRegion(*clipPtr);
        }
        return;
    }

    SkScanClipper clipper(blitter, clipPtr, ir, path.isInverseFillType(), irPreClipped);

    blitter = clipper.getBlitter();
    if (blitter) {
        // we have to keep our calls to blitter in sorted order, so we
        // must blit the above section first, then the middle, then the bottom.
        if (path.isInverseFillType()) {
            sk_blit_above(blitter, ir, *clipPtr);
        }
        SkASSERT(clipper.getClipRect() == nullptr ||
                *clipper.getClipRect() == clipPtr->getBounds());
        sk_fill_path(path, clipPtr->getBounds(), blitter, ir.fTop, ir.fBottom,
                     clipper.getClipRect() == nullptr);
        if (path.isInverseFillType()) {
            sk_blit_below(blitter, ir, *clipPtr);
        }
    } else {
        // what does it mean to not have a blitter if path.isInverseFillType???
    }
}

bool SkScan::PathRequiresTiling(const SkIRect& bounds) {
    SkRegion out;  // ignored
    return clip_to_limit(SkRegion(bounds), &out);
}

///////////////////////////////////////////////////////////////////////////////

static int build_tri_edges(SkEdge edge[], const SkPoint pts[],
                           const SkIRect* clipRect, SkEdge* list[]) {
    SkEdge** start = list;

    if (edge->setLine(pts[0], pts[1], clipRect)) {
        *list++ = edge;
        edge++;
    }
    if (edge->setLine(pts[1], pts[2], clipRect)) {
        *list++ = edge;
        edge++;
    }
    if (edge->setLine(pts[2], pts[0], clipRect)) {
        *list++ = edge;
    }
    return (int)(list - start);
}


static void sk_fill_triangle(const SkPoint pts[], const SkIRect* clipRect,
                             SkBlitter* blitter, const SkIRect& ir) {
    SkASSERT(pts && blitter);

    SkEdge edgeStorage[3];
    SkEdge* list[3];

    int count = build_tri_edges(edgeStorage, pts, clipRect, list);
    if (count < 2) {
        return;
    }

    SkEdge headEdge, tailEdge, *last;

    // this returns the first and last edge after they're sorted into a dlink list
    SkEdge* edge = sort_edges(list, count, &last);

    headEdge.fPrev = nullptr;
    headEdge.fNext = edge;
    headEdge.fFirstY = kEDGE_HEAD_Y;
    headEdge.fX = SK_MinS32;
    edge->fPrev = &headEdge;

    tailEdge.fPrev = last;
    tailEdge.fNext = nullptr;
    tailEdge.fFirstY = kEDGE_TAIL_Y;
    last->fNext = &tailEdge;

    // now edge is the head of the sorted linklist
    int stop_y = ir.fBottom;
    if (clipRect && stop_y > clipRect->fBottom) {
        stop_y = clipRect->fBottom;
    }
    int start_y = ir.fTop;
    if (clipRect && start_y < clipRect->fTop) {
        start_y = clipRect->fTop;
    }
    walk_simple_edges(&headEdge, blitter, start_y, stop_y);
}

void SkScan::FillTriangle(const SkPoint pts[], const SkRasterClip& clip,
                          SkBlitter* blitter) {
    if (clip.isEmpty()) {
        return;
    }

    const auto r = SkRect::BoundsOrEmpty({pts, 3});
    // If r is too large (larger than can easily fit in SkFixed) then we need perform geometric
    // clipping. This is a bit of work, so we just call the general FillPath() to handle it.
    // Use FixedMax/2 as the limit so we can subtract two edges and still store that in Fixed.
    const SkScalar limit = SK_MaxS16 >> 1;
    if (!SkRect::MakeLTRB(-limit, -limit, limit, limit).contains(r)) {
        FillPath(SkPath::Polygon({pts, 3}, false), clip, blitter);
        return;
    }

    SkIRect ir = conservative_round_to_int(r);
    if (ir.isEmpty() || !SkIRect::Intersects(ir, clip.getBounds())) {
        return;
    }

    SkAAClipBlitterWrapper wrap;
    const SkRegion* clipRgn;
    if (clip.isBW()) {
        clipRgn = &clip.bwRgn();
    } else {
        wrap.init(clip, blitter);
        clipRgn = &wrap.getRgn();
        blitter = wrap.getBlitter();
    }

    SkScanClipper clipper(blitter, clipRgn, ir);
    blitter = clipper.getBlitter();
    if (blitter) {
        sk_fill_triangle(pts, clipper.getClipRect(), blitter, ir);
    }
}
