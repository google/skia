/*
 * Copyright 2016 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnalyticEdge.h"
#include "SkAntiRun.h"
#include "SkAutoMalloc.h"
#include "SkBlitter.h"
#include "SkCoverageDelta.h"
#include "SkEdge.h"
#include "SkEdgeBuilder.h"
#include "SkGeometry.h"
#include "SkMask.h"
#include "SkPath.h"
#include "SkQuadClipper.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "SkScan.h"
#include "SkScanPriv.h"
#include "SkTSort.h"
#include "SkTemplates.h"
#include "SkUTF.h"

#if defined(SK_DISABLE_DAA)
void SkScan::DAAFillPath(const SkPath& path, SkBlitter* blitter, const SkIRect& ir,
                         const SkIRect& clipBounds, bool forceRLE, SkDAARecord* record) {
    SkDEBUGFAIL("DAA Disabled");
    return;
}
#else
///////////////////////////////////////////////////////////////////////////////

/*

DAA stands for Delta-based Anti-Aliasing.

This is an improved version of our analytic AA algorithm (in SkScan_AAAPath.cpp)
which first scan convert paths into coverage deltas (this step can happen out of order,
and we don't seem to be needed to worry about the intersection, clamping, etc.),
and then use a single blitter run to convert all those deltas into the final alphas.

Before we go to the final blitter run, we'll use SkFixed for all delta values so we
don't ever have to worry about under/overflow.

*/

///////////////////////////////////////////////////////////////////////////////

// The following helper functions are the same as those from SkScan_AAAPath.cpp
// except that we use SkFixed everywhere instead of SkAlpha.

static inline SkFixed SkFixedMul_lowprec(SkFixed a, SkFixed b) {
    return (a >> 8) * (b >> 8);
}

// Return the alpha of a trapezoid whose height is 1
static inline SkFixed trapezoidToAlpha(SkFixed l1, SkFixed l2) {
    SkASSERT(l1 >= 0 && l2 >= 0);
    return (l1 + l2) >> 1;
}

// The alpha of right-triangle (a, a*b)
static inline SkFixed partialTriangleToAlpha(SkFixed a, SkFixed b) {
    SkASSERT(a <= SK_Fixed1);
    // SkFixedMul(SkFixedMul(a, a), b) >> 1
    // return ((((a >> 8) * (a >> 8)) >> 8) * (b >> 8)) >> 1;
    return (a >> 11) * (a >> 11) * (b >> 11);
}

static inline SkFixed getPartialAlpha(SkFixed alpha, SkFixed partialMultiplier) {
    // DAA should not be so sensitive to the precision (compared to AAA).
    return SkFixedMul_lowprec(alpha, partialMultiplier);
}

///////////////////////////////////////////////////////////////////////////////

template<bool isPartial, class Deltas>
static inline void add_coverage_delta_segment(int y, SkFixed rowHeight, const SkAnalyticEdge* edge,
        SkFixed nextX, Deltas* deltas) { // rowHeight=fullAlpha
    SkASSERT(rowHeight <= SK_Fixed1 && rowHeight >= 0);

    // Let's see if multiplying sign is faster than multiplying edge->fWinding.
    // (Compiler should be able to optimize multiplication with 1/-1?)
    int sign = edge->fWinding == 1 ? 1 : -1;

    SkFixed l = SkTMin(edge->fX, nextX);
    SkFixed r = edge->fX + nextX - l;
    int     L = SkFixedFloorToInt(l);
    int     R = SkFixedCeilToInt(r);
    int     len = R - L;

    switch (len) {
        case 0: {
            deltas->addDelta(L, y, rowHeight * sign);
            return;
        }
        case 1: {
            SkFixed fixedR  = SkIntToFixed(R);
            SkFixed alpha   = trapezoidToAlpha(fixedR - l, fixedR - r);
            if (isPartial) {
                alpha = getPartialAlpha(alpha, rowHeight);
            }
            deltas->addDelta(L,     y,  alpha * sign);
            deltas->addDelta(L + 1, y,  (rowHeight - alpha) * sign);
            return;
        }
        case 2: {
            SkFixed middle  = SkIntToFixed(L + 1);
            SkFixed x1      = middle - l;
            SkFixed x2      = r - middle;
            SkFixed alpha1  = partialTriangleToAlpha(x1, edge->fDY);
            SkFixed alpha2  = rowHeight - partialTriangleToAlpha(x2, edge->fDY);
            deltas->addDelta(L,     y,  alpha1 * sign);
            deltas->addDelta(L + 1, y,  (alpha2 - alpha1) * sign);
            deltas->addDelta(L + 2, y,  (rowHeight - alpha2) * sign);
            return;
        }
    }

    // When len > 2, computations are similar to computeAlphaAboveLine in SkScan_AAAPath.cpp
    SkFixed dY      = edge->fDY;
    SkFixed fixedL  = SkIntToFixed(L);
    SkFixed fixedR  = SkIntToFixed(R);
    SkFixed first   = SK_Fixed1 + fixedL - l; // horizontal edge of the left-most triangle
    SkFixed last    = r - (fixedR - SK_Fixed1); // horizontal edge of the right-most triangle
    SkFixed firstH  = SkFixedMul_lowprec(first, dY); // vertical edge of the left-most triangle

    SkFixed alpha0  = SkFixedMul_lowprec(first, firstH) >> 1;   // triangle alpha
    SkFixed alpha1  = firstH + (dY >> 1);                       // rectangle plus triangle
    deltas->addDelta(L, y, alpha0 * sign);
    deltas->addDelta(L + 1, y, (alpha1 - alpha0) * sign);
    for(int i = 2; i < len - 1; ++i) {
        deltas->addDelta(L + i, y, dY * sign); // the increment is always a rect of height = dY
    }

    SkFixed alphaR2     = alpha1 + dY * (len - 3);                      // the alpha at R - 2
    SkFixed lastAlpha   = rowHeight - partialTriangleToAlpha(last, dY); // the alpha at R - 1
    deltas->addDelta(R - 1, y, (lastAlpha - alphaR2) * sign);
    deltas->addDelta(R,     y, (rowHeight - lastAlpha) * sign);
}

class XLessThan {
public:
    bool operator()(const SkBezier* a, const SkBezier* b) {
        return a->fP0.fX + a->fP1.fX < b->fP0.fX + b->fP1.fX;
    }
};

class YLessThan {
public:
    bool operator()(const SkBezier* a, const SkBezier* b) {
        return a->fP0.fY + a->fP1.fY < b->fP0.fY + b->fP1.fY;
    }
};

template<class Deltas> static SK_ALWAYS_INLINE
void gen_alpha_deltas(const SkPath& path, const SkIRect& clippedIR, const SkIRect& clipBounds,
        Deltas& result, SkBlitter* blitter, bool skipRect, bool pathContainedInClip) {
    // 1. Build edges
    SkBezierEdgeBuilder builder;
    // We have to use clipBounds instead of clippedIR to build edges because of "canCullToTheRight":
    // if the builder finds a right edge past the right clip, it won't build that right edge.
    int  count = builder.buildEdges(path, pathContainedInClip ? nullptr : &clipBounds);

    if (count == 0) {
        return;
    }
    SkBezier** list = builder.bezierList();

    // 2. Try to find the rect part because blitAntiRect is so much faster than blitCoverageDeltas
    int rectTop = clippedIR.fBottom;   // the rect is initialized to be empty as top = bot
    int rectBot = clippedIR.fBottom;
    if (skipRect) {             // only find that rect is skipRect == true
        YLessThan lessThan;     // sort edges in YX order
        SkTQSort(list, list + count - 1, lessThan);
        for(int i = 0; i < count - 1; ++i) {
            SkBezier* lb = list[i];
            SkBezier* rb = list[i + 1];

            // fCount == 2 ensures that lb and rb are lines instead of quads or cubics.
            bool lDX0 = lb->fP0.fX == lb->fP1.fX && lb->fCount == 2;
            bool rDX0 = rb->fP0.fX == rb->fP1.fX && rb->fCount == 2;
            if (!lDX0 || !rDX0) { // make sure that the edges are vertical
                continue;
            }

            SkAnalyticEdge l, r;
            if (!l.setLine(lb->fP0, lb->fP1) || !r.setLine(rb->fP0, rb->fP1)) {
                continue;
            }

            SkFixed xorUpperY = l.fUpperY ^ r.fUpperY;
            SkFixed xorLowerY = l.fLowerY ^ r.fLowerY;
            if ((xorUpperY | xorLowerY) == 0) { // equal upperY and lowerY
                rectTop = SkFixedCeilToInt(l.fUpperY);
                rectBot = SkFixedFloorToInt(l.fLowerY);
                if (rectBot > rectTop) { // if bot == top, the rect is too short for blitAntiRect
                    int L = SkFixedCeilToInt(l.fUpperX);
                    int R = SkFixedFloorToInt(r.fUpperX);
                    if (L <= R) {
                        SkAlpha la = (SkIntToFixed(L) - l.fUpperX) >> 8;
                        SkAlpha ra = (r.fUpperX - SkIntToFixed(R)) >> 8;
                        result.setAntiRect(L - 1, rectTop, R - L, rectBot - rectTop, la, ra);
                    } else { // too thin to use blitAntiRect; reset the rect region to be emtpy
                        rectTop = rectBot = clippedIR.fBottom;
                    }
                }
                break;
            }

        }
    }

    // 3. Sort edges in x so we may need less sorting for delta based on x. This only helps
    //    SkCoverageDeltaList. And we don't want to sort more than SORT_THRESHOLD edges where
    //    the log(count) factor of the quick sort may become a bottleneck; when there are so
    //    many edges, we're unlikely to make deltas sorted anyway.
    constexpr int SORT_THRESHOLD = 256;
    if (std::is_same<Deltas, SkCoverageDeltaList>::value && count < SORT_THRESHOLD) {
        XLessThan lessThan;
        SkTQSort(list, list + count - 1, lessThan);
    }

    // Future todo: parallize and SIMD the following code.
    // 4. iterate through edges and generate deltas
    for(int index = 0; index < count; ++index) {
        SkAnalyticCubicEdge storage;
        SkASSERT(sizeof(SkAnalyticQuadraticEdge) >= sizeof(SkAnalyticEdge));
        SkASSERT(sizeof(SkAnalyticCubicEdge) >= sizeof(SkAnalyticQuadraticEdge));

        SkBezier* bezier        = list[index];
        SkAnalyticEdge* currE   = &storage;
        bool edgeSet            = false;

        int originalWinding = 1;
        bool sortY = true;
        switch (bezier->fCount) {
            case 2: {
                edgeSet = currE->setLine(bezier->fP0, bezier->fP1);
                originalWinding = currE->fWinding;
                break;
            }
            case 3: {
                SkQuad* quad = static_cast<SkQuad*>(bezier);
                SkPoint pts[3] = {quad->fP0, quad->fP1, quad->fP2};
                edgeSet = static_cast<SkAnalyticQuadraticEdge*>(currE)->setQuadratic(pts);
                originalWinding = static_cast<SkAnalyticQuadraticEdge*>(currE)->fQEdge.fWinding;
                break;
            }
            case 4: {
                sortY = false;
                SkCubic* cubic = static_cast<SkCubic*>(bezier);
                SkPoint pts[4] = {cubic->fP0, cubic->fP1, cubic->fP2, cubic->fP3};
                edgeSet = static_cast<SkAnalyticCubicEdge*>(currE)->setCubic(pts, sortY);
                originalWinding = static_cast<SkAnalyticCubicEdge*>(currE)->fCEdge.fWinding;
                break;
            }
        }

        if (!edgeSet) {
            continue;
        }

        do {
            currE->fX =  currE->fUpperX;

            SkFixed upperFloor  = SkFixedFloorToFixed(currE->fUpperY);
            SkFixed lowerCeil   = SkFixedCeilToFixed(currE->fLowerY);
            int     iy          = SkFixedFloorToInt(upperFloor);

            if (lowerCeil <= upperFloor + SK_Fixed1) { // only one row is affected by the currE
                SkFixed rowHeight = currE->fLowerY - currE->fUpperY;
                SkFixed nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                if (iy >= clippedIR.fTop && iy < clippedIR.fBottom) {
                    add_coverage_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
                }
                continue;
            }

            // check first row
            SkFixed rowHeight = upperFloor + SK_Fixed1 - currE->fUpperY;
            SkFixed nextX;
            if (rowHeight != SK_Fixed1) {   // it's a partial row
                nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_coverage_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
            } else {                        // it's a full row so we can leave it to the while loop
                iy--;                       // compensate the iy++ in the while loop
                nextX = currE->fX;
            }

            while (true) { // process the full rows in the middle
                iy++;
                SkFixed y = SkIntToFixed(iy);
                currE->fX = nextX;
                nextX += currE->fDX;

                if (y + SK_Fixed1 > currE->fLowerY) {
                    break; // no full rows left, break
                }

                // Check whether we're in the rect part that will be covered by blitAntiRect
                if (iy >= rectTop && iy < rectBot) {
                    SkASSERT(currE->fDX == 0);  // If yes, we must be on an edge with fDX = 0.
                    iy = rectBot - 1;           // Skip the rect part by advancing iy to the bottom.
                    continue;
                }

                // Add current edge's coverage deltas on this full row
                add_coverage_delta_segment<false>(iy, SK_Fixed1, currE, nextX, &result);
            }

            // last partial row
            if (SkIntToFixed(iy) < currE->fLowerY &&
                    iy >= clippedIR.fTop && iy < clippedIR.fBottom) {
                rowHeight = currE->fLowerY - SkIntToFixed(iy);
                nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_coverage_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
            }
        // Intended assignment to fWinding to restore the maybe-negated winding (during updateLine)
        } while ((currE->fWinding = originalWinding) && currE->update(currE->fLowerY, sortY));
    }
}

void SkScan::DAAFillPath(const SkPath& path, SkBlitter* blitter, const SkIRect& ir,
                         const SkIRect& clipBounds, bool forceRLE, SkDAARecord* record) {
    bool containedInClip = clipBounds.contains(ir);
    bool isEvenOdd  = path.getFillType() & 1;
    bool isConvex   = path.isConvex();
    bool isInverse  = path.isInverseFillType();
    bool skipRect   = isConvex && !isInverse;
    bool isInitOnce = record && record->fType == SkDAARecord::Type::kToBeComputed;

    SkIRect clippedIR = ir;
    clippedIR.intersect(clipBounds);

    // The overhead of even constructing SkCoverageDeltaList/Mask is too big.
    // So TryBlitFatAntiRect and return if it's successful.
    if (!isInverse && TryBlitFatAntiRect(blitter, path, clipBounds)) {
        SkDAARecord::SetEmpty(record);
        return;
    }

#ifdef SK_BUILD_FOR_GOOGLE3
    constexpr int STACK_SIZE = 12 << 10; // 12K stack size alloc; Google3 has 16K limit.
#else
    constexpr int STACK_SIZE = 64 << 10; // 64k stack size to avoid heap allocation
#endif
    SkSTArenaAlloc<STACK_SIZE> stackAlloc; // avoid heap allocation with SkSTArenaAlloc

    // Set alloc to record's alloc if and only if we're in the init-once phase. We have to do that
    // during init phase because the mask or list needs to live longer. We can't do that during blit
    // phase because the same record could be accessed by multiple threads simultaneously.
    SkArenaAlloc* alloc = isInitOnce ? record->fAlloc : &stackAlloc;

    if (record == nullptr) {
        record = alloc->make<SkDAARecord>(alloc);
    }

    // Only blitter->blitXXX needs to be done in order in the threaded backend. Everything else can
    // be done out of order in the init-once phase. We do that by calling DAAFillPath twice: first
    // with a null blitter, and then second with the real blitter and the SkMask/SkCoverageDeltaList
    // generated in the first step.
    if (record->fType == SkDAARecord::Type::kToBeComputed) {
        if (!forceRLE && !isInverse && SkCoverageDeltaMask::Suitable(clippedIR)) {
            record->fType = SkDAARecord::Type::kMask;
            SkCoverageDeltaMask deltaMask(alloc, clippedIR);
            gen_alpha_deltas(path, clippedIR, clipBounds, deltaMask, blitter, skipRect,
                             containedInClip);
            deltaMask.convertCoverageToAlpha(isEvenOdd, isInverse, isConvex);
            record->fMask = deltaMask.prepareSkMask();
        } else {
            record->fType = SkDAARecord::Type::kList;
            SkCoverageDeltaList* deltaList = alloc->make<SkCoverageDeltaList>(
                    alloc, clippedIR, forceRLE);
            gen_alpha_deltas(path, clippedIR, clipBounds, *deltaList, blitter, skipRect,
                             containedInClip);
            record->fList = deltaList;
        }
    }

    if (!isInitOnce) {
        SkASSERT(record->fType != SkDAARecord::Type::kToBeComputed);
        if (record->fType == SkDAARecord::Type::kMask) {
            blitter->blitMask(record->fMask, clippedIR);
        } else {
            blitter->blitCoverageDeltas(record->fList, clipBounds, isEvenOdd, isInverse, isConvex);
        }
    }
}
#endif //defined(SK_DISABLE_DAA)
