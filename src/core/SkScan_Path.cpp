/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScanPriv.h"
#include "SkBlitter.h"
#include "SkEdge.h"
#include "SkEdgeBuilder.h"
#include "SkGeometry.h"
#include "SkPath.h"
#include "SkQuadClipper.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "SkTemplates.h"
#include "SkTSort.h"

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

static void walk_edges(SkEdge* prevHead, SkPath::FillType fillType,
                       SkBlitter* blitter, int start_y, int stop_y,
                       PrePostProc proc, int rightClip) {
    validate_sort(prevHead->fNext);

    int curr_y = start_y;
    // returns 1 for evenodd, -1 for winding, regardless of inverse-ness
    int windingMask = (fillType & 1) ? 1 : -1;

    for (;;) {
        int     w = 0;
        int     left SK_INIT_TO_AVOID_WARNING;
        bool    in_interval = false;
        SkEdge* currE = prevHead->fNext;
        SkFixed prevX = prevHead->fX;

        validate_edges_for_y(currE, curr_y);

        if (proc) {
            proc(blitter, curr_y, PREPOST_START);    // pre-proc
        }

        while (currE->fFirstY <= curr_y) {
            SkASSERT(currE->fLastY >= curr_y);

            int x = SkFixedRoundToInt(currE->fX);
            w += currE->fWinding;
            if ((w & windingMask) == 0) { // we finished an interval
                SkASSERT(in_interval);
                int width = x - left;
                SkASSERT(width >= 0);
                if (width)
                    blitter->blitH(left, curr_y, width);
                in_interval = false;
            } else if (!in_interval) {
                left = x;
                in_interval = true;
            }

            SkEdge* next = currE->fNext;
            SkFixed newX;

            if (currE->fLastY == curr_y) {    // are we done with this edge?
                if (currE->fCurveCount < 0) {
                    if (((SkCubicEdge*)currE)->updateCubic()) {
                        SkASSERT(currE->fFirstY == curr_y + 1);

                        newX = currE->fX;
                        goto NEXT_X;
                    }
                } else if (currE->fCurveCount > 0) {
                    if (((SkQuadraticEdge*)currE)->updateQuadratic()) {
                        newX = currE->fX;
                        goto NEXT_X;
                    }
                }
                remove_edge(currE);
            } else {
                SkASSERT(currE->fLastY > curr_y);
                newX = currE->fX + currE->fDX;
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

        // was our right-edge culled away?
        if (in_interval) {
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

// return true if we're done with this edge
static bool update_edge(SkEdge* edge, int last_y) {
    SkASSERT(edge->fLastY >= last_y);
    if (last_y == edge->fLastY) {
        if (edge->fCurveCount < 0) {
            if (((SkCubicEdge*)edge)->updateCubic()) {
                SkASSERT(edge->fFirstY == last_y + 1);
                return false;
            }
        } else if (edge->fCurveCount > 0) {
            if (((SkQuadraticEdge*)edge)->updateQuadratic()) {
                SkASSERT(edge->fFirstY == last_y + 1);
                return false;
            }
        }
        return true;
    }
    return false;
}

static void walk_convex_edges(SkEdge* prevHead, SkPath::FillType,
                              SkBlitter* blitter, int start_y, int stop_y,
                              PrePostProc proc) {
    validate_sort(prevHead->fNext);

    SkEdge* leftE = prevHead->fNext;
    SkEdge* riteE = leftE->fNext;
    SkEdge* currE = riteE->fNext;

#if 0
    int local_top = leftE->fFirstY;
    SkASSERT(local_top == riteE->fFirstY);
#else
    // our edge choppers for curves can result in the initial edges
    // not lining up, so we take the max.
    int local_top = SkMax32(leftE->fFirstY, riteE->fFirstY);
#endif
    SkASSERT(local_top >= start_y);

    for (;;) {
        SkASSERT(leftE->fFirstY <= stop_y);
        SkASSERT(riteE->fFirstY <= stop_y);

        if (leftE->fX > riteE->fX || (leftE->fX == riteE->fX &&
                                      leftE->fDX > riteE->fDX)) {
            SkTSwap(leftE, riteE);
        }

        int local_bot = SkMin32(leftE->fLastY, riteE->fLastY);
        local_bot = SkMin32(local_bot, stop_y - 1);
        SkASSERT(local_top <= local_bot);

        SkFixed left = leftE->fX;
        SkFixed dLeft = leftE->fDX;
        SkFixed rite = riteE->fX;
        SkFixed dRite = riteE->fDX;
        int count = local_bot - local_top;
        SkASSERT(count >= 0);
        if (0 == (dLeft | dRite)) {
            int L = SkFixedRoundToInt(left);
            int R = SkFixedRoundToInt(rite);
            if (L < R) {
                count += 1;
                blitter->blitRect(L, local_top, R - L, count);
            }
            local_top = local_bot + 1;
        } else {
            do {
                int L = SkFixedRoundToInt(left);
                int R = SkFixedRoundToInt(rite);
                if (L < R) {
                    blitter->blitH(L, local_top, R - L);
                }
                left += dLeft;
                rite += dRite;
                local_top += 1;
            } while (--count >= 0);
        }

        leftE->fX = left;
        riteE->fX = rite;

        if (update_edge(leftE, local_bot)) {
            if (currE->fFirstY >= stop_y) {
                break;
            }
            leftE = currE;
            currE = currE->fNext;
        }
        if (update_edge(riteE, local_bot)) {
            if (currE->fFirstY >= stop_y) {
                break;
            }
            riteE = currE;
            currE = currE->fNext;
        }

        SkASSERT(leftE);
        SkASSERT(riteE);

        // check our bottom clip
        SkASSERT(local_top == local_bot + 1);
        if (local_top >= stop_y) {
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

// this guy overrides blitH, and will call its proxy blitter with the inverse
// of the spans it is given (clipped to the left/right of the cliprect)
//
// used to implement inverse filltypes on paths
//
class InverseBlitter : public SkBlitter {
public:
    void setBlitter(SkBlitter* blitter, const SkIRect& clip, int shift) {
        fBlitter = blitter;
        fFirstX = clip.fLeft << shift;
        fLastX = clip.fRight << shift;
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
    const SkPixmap* justAnOpaqueColor(uint32_t* value) override {
        SkDEBUGFAIL("justAnOpaqueColor unexpected");
        return nullptr;
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

static bool operator<(const SkEdge& a, const SkEdge& b) {
    int valuea = a.fFirstY;
    int valueb = b.fFirstY;

    if (valuea == valueb) {
        valuea = a.fX;
        valueb = b.fX;
    }

    return valuea < valueb;
}

static SkEdge* sort_edges(SkEdge* list[], int count, SkEdge** last) {
    SkTQSort(list, list + count - 1);

    // now make the edges linked in sorted order
    for (int i = 1; i < count; i++) {
        list[i - 1]->fNext = list[i];
        list[i]->fPrev = list[i - 1];
    }

    *last = list[count - 1];
    return list[0];
}

// clipRect has not been shifted up
void sk_fill_path(const SkPath& path, const SkIRect& clipRect, SkBlitter* blitter,
                  int start_y, int stop_y, int shiftEdgesUp, bool pathContainedInClip) {
    SkASSERT(blitter);

    SkIRect shiftedClip = clipRect;
    shiftedClip.fLeft <<= shiftEdgesUp;
    shiftedClip.fRight <<= shiftEdgesUp;
    shiftedClip.fTop <<= shiftEdgesUp;
    shiftedClip.fBottom <<= shiftEdgesUp;

    SkEdgeBuilder builder;
    int count = builder.build_edges(path, &shiftedClip, shiftEdgesUp, pathContainedInClip);
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
                blitter->blitRect(rect.fLeft << shiftEdgesUp,
                                  rect.fTop << shiftEdgesUp,
                                  rect.width() << shiftEdgesUp,
                                  rect.height() << shiftEdgesUp);
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

    start_y = SkLeftShift(start_y, shiftEdgesUp);
    stop_y = SkLeftShift(stop_y, shiftEdgesUp);
    if (!pathContainedInClip && start_y < shiftedClip.fTop) {
        start_y = shiftedClip.fTop;
    }
    if (!pathContainedInClip && stop_y > shiftedClip.fBottom) {
        stop_y = shiftedClip.fBottom;
    }

    InverseBlitter  ib;
    PrePostProc     proc = nullptr;

    if (path.isInverseFillType()) {
        ib.setBlitter(blitter, clipRect, shiftEdgesUp);
        blitter = &ib;
        proc = PrePostInverseBlitterProc;
    }

    // count >= 2 is required as the convex walker does not handle missing right edges
    if (path.isConvex() && (nullptr == proc) && count >= 2) {
        walk_convex_edges(&headEdge, path.getFillType(), blitter, start_y, stop_y, nullptr);
    } else {
        walk_edges(&headEdge, path.getFillType(), blitter, start_y, stop_y, proc,
                shiftedClip.right());
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
                             const SkIRect& ir, bool skipRejectTest) {
    fBlitter = nullptr;     // null means blit nothing
    fClipRect = nullptr;

    if (clip) {
        fClipRect = &clip->getBounds();
        if (!skipRejectTest && !SkIRect::Intersects(*fClipRect, ir)) { // completely clipped out
            return;
        }

        if (clip->isRect()) {
            if (fClipRect->contains(ir)) {
#ifdef SK_DEBUG
                fRectClipCheckBlitter.init(blitter, *fClipRect);
                blitter = &fRectClipCheckBlitter;
#endif
                fClipRect = nullptr;
            } else {
                // only need a wrapper blitter if we're horizontally clipped
                if (fClipRect->fLeft > ir.fLeft || fClipRect->fRight < ir.fRight) {
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
    const int32_t limit = 32767;

    SkIRect limitR;
    limitR.set(-limit, -limit, limit, limit);
    if (limitR.contains(orig.getBounds())) {
        return false;
    }
    reduced->op(orig, limitR, SkRegion::kIntersect_Op);
    return true;
}

/**
  * Variants of SkScalarRoundToInt, identical to SkDScalarRoundToInt except when the input fraction
  * is 0.5. When SK_RASTERIZE_EVEN_ROUNDING is enabled, we must bias the result before rounding to
  * account for potential FDot6 rounding edge-cases.
  */
#ifdef SK_RASTERIZE_EVEN_ROUNDING
static const double kRoundBias = 0.5 / SK_FDot6One;
#else
static const double kRoundBias = 0.0;
#endif

/**
  * Round the value down. This is used to round the top and left of a rectangle,
  * and corresponds to the way the scan converter treats the top and left edges.
  */
static inline int round_down_to_int(SkScalar x) {
    double xx = x;
    xx -= 0.5 + kRoundBias;
    return (int)ceil(xx);
}

/**
  * Round the value up. This is used to round the bottom and right of a rectangle,
  * and corresponds to the way the scan converter treats the bottom and right edges.
  */
static inline int round_up_to_int(SkScalar x) {
    double xx = x;
    xx += 0.5 + kRoundBias;
    return (int)floor(xx);
}

/**
  *  Variant of SkRect::round() that explicitly performs the rounding step (i.e. floor(x + 0.5))
  *  using double instead of SkScalar (float). It does this by calling SkDScalarRoundToInt(),
  *  which may be slower than calling SkScalarRountToInt(), but gives slightly more accurate
  *  results. Also rounds top and left using double, flooring when the fraction is exactly 0.5f.
  *
  *  e.g.
  *      SkScalar left = 0.5f;
  *      int ileft = SkScalarRoundToInt(left);
  *      SkASSERT(0 == ileft);  // <--- fails
  *      int ileft = round_down_to_int(left);
  *      SkASSERT(0 == ileft);  // <--- succeeds
  *      SkScalar right = 0.49999997f;
  *      int iright = SkScalarRoundToInt(right);
  *      SkASSERT(0 == iright);  // <--- fails
  *      iright = SkDScalarRoundToInt(right);
  *      SkASSERT(0 == iright);  // <--- succeeds
  *
  *
  *  If using SK_RASTERIZE_EVEN_ROUNDING, we need to ensure we account for edges bounded by this
  *  rect being rounded to FDot6 format before being later rounded to an integer. For example, a
  *  value like 0.499 can be below 0.5, but round to 0.5 as FDot6, which would finally round to
  *  the integer 1, instead of just rounding to 0.
  *
  *  To handle this, a small bias of half an FDot6 increment is added before actually rounding to
  *  an integer value. This simulates the rounding of SkScalarRoundToFDot6 without incurring the
  *  range loss of converting to FDot6 format first, preserving the integer range for the SkIRect.
  *  Thus, bottom and right are rounded in this manner (biased up), ensuring the rect is large
  *  enough.
  */
static void round_asymmetric_to_int(const SkRect& src, SkIRect* dst) {
    SkASSERT(dst);
    dst->set(round_down_to_int(src.fLeft), round_down_to_int(src.fTop),
             round_up_to_int(src.fRight), round_up_to_int(src.fBottom));
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

    SkIRect ir;
    // We deliberately call round_asymmetric_to_int() instead of round(), since we can't afford
    // to generate a bounds that is tighter than the corresponding SkEdges. The edge code basically
    // converts the floats to fixed, and then "rounds". If we called round() instead of
    // round_asymmetric_to_int() here, we could generate the wrong ir for values like 0.4999997.
    round_asymmetric_to_int(path.getBounds(), &ir);
    if (ir.isEmpty()) {
        if (path.isInverseFillType()) {
            blitter->blitRegion(*clipPtr);
        }
        return;
    }

    SkScanClipper clipper(blitter, clipPtr, ir, path.isInverseFillType());

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
                     0, clipper.getClipRect() == nullptr);
        if (path.isInverseFillType()) {
            sk_blit_below(blitter, ir, *clipPtr);
        }
    } else {
        // what does it mean to not have a blitter if path.isInverseFillType???
    }
}

void SkScan::FillPath(const SkPath& path, const SkIRect& ir,
                      SkBlitter* blitter) {
    SkRegion rgn(ir);
    FillPath(path, rgn, blitter);
}

///////////////////////////////////////////////////////////////////////////////

static int build_tri_edges(SkEdge edge[], const SkPoint pts[],
                           const SkIRect* clipRect, SkEdge* list[]) {
    SkEdge** start = list;

    if (edge->setLine(pts[0], pts[1], clipRect, 0)) {
        *list++ = edge;
        edge = (SkEdge*)((char*)edge + sizeof(SkEdge));
    }
    if (edge->setLine(pts[1], pts[2], clipRect, 0)) {
        *list++ = edge;
        edge = (SkEdge*)((char*)edge + sizeof(SkEdge));
    }
    if (edge->setLine(pts[2], pts[0], clipRect, 0)) {
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
    walk_convex_edges(&headEdge, SkPath::kEvenOdd_FillType, blitter, start_y, stop_y, nullptr);
//    walk_edges(&headEdge, SkPath::kEvenOdd_FillType, blitter, start_y, stop_y, nullptr);
}

void SkScan::FillTriangle(const SkPoint pts[], const SkRasterClip& clip,
                          SkBlitter* blitter) {
    if (clip.isEmpty()) {
        return;
    }

    SkRect  r;
    SkIRect ir;
    r.set(pts, 3);
    r.round(&ir);
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
