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
#include "SkUtils.h"

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
    bool operator()(const SkAnalyticEdge* a, const SkAnalyticEdge* b) {
        return a->fUpperX < b->fUpperX || (a->fUpperX == b->fUpperX && a->fDX < b->fDX);
    }
};

class YXLessThan {
public:
    bool operator()(const SkAnalyticEdge* a, const SkAnalyticEdge* b) {
        return (a->fUpperY < b->fUpperY) || (a->fUpperY == b->fUpperY && a->fUpperX < b->fUpperX);
    }
};

template<class Deltas>
static void gen_alpha_deltas(const SkPath& path, const SkRegion& clipRgn, Deltas& result,
        SkBlitter* blitter, bool skipRect, bool pathContainedInClip) {
    // 1. Build edges
    SkEdgeBuilder builder;
    SkIRect ir               = path.getBounds().roundOut();
    const SkIRect& clipRect  = clipRgn.getBounds();
    int  count               = builder.build_edges(path, &clipRect, 0, pathContainedInClip, true);
    if (count == 0) {
        return;
    }
    SkAnalyticEdge** list = builder.analyticEdgeList();

    // 2. Try to find the rect part because blitAntiRect is so much faster than blitCoverageDeltas
    int rectTop = ir.fBottom;   // the rect is initialized to be empty as top = bot
    int rectBot = ir.fBottom;
    if (skipRect) {             // only find that rect is skipRect == true
        YXLessThan lessThan;    // sort edges in YX order
        SkTQSort(list, list + count - 1, lessThan);
        for(int i = 0; i < count - 1; ++i) {
            SkAnalyticEdge* l = list[i];
            SkAnalyticEdge* r = list[i + 1];
            SkFixed xorUpperY = l->fUpperY ^ r->fUpperY;
            SkFixed xorLowerY = l->fLowerY ^ r->fLowerY;
            SkFixed orDX      = l->fDX | r->fDX;
            if ((xorUpperY | xorLowerY | orDX) == 0) { // two edges with identical y and zero dx
                rectTop = SkFixedCeilToInt(l->fUpperY);
                rectBot = SkFixedFloorToInt(l->fLowerY);
                if (rectBot > rectTop) { // if bot == top, the rect is too short for blitAntiRect
                    int L = SkFixedCeilToInt(l->fUpperX);
                    int R = SkFixedFloorToInt(r->fUpperX);
                    if (L <= R) {
                        SkAlpha la = (SkIntToFixed(L) - l->fUpperX) >> 8;
                        SkAlpha ra = (r->fUpperX - SkIntToFixed(R)) >> 8;
                        result.setAntiRect(L - 1, rectTop, R - L, rectBot - rectTop, la, ra);
                    } else { // too thin to use blitAntiRect; reset the rect region to be emtpy
                        rectTop = rectBot = ir.fBottom;
                    }
                }
                break;
            }
        }
    }

    // 3. Sort edges in x so we may need less sorting for delta based on x
    XLessThan lessThan;
    SkTQSort(list, list + count - 1, lessThan);

    // Future todo: parallize and SIMD the following code.
    // 4. iterate through edges and generate deltas
    for(int edgeIndex = 0; edgeIndex < count; ++edgeIndex) {
        // Future todo: SkAnalyticEdge has several fields (e.g., fNext, fPrev, fRiteE...) that are
        // useless here. Remove them may bring performance improvement.
        SkAnalyticEdge* currE = list[edgeIndex];

        do {
            currE->fX =  currE->fUpperX;

            SkFixed upperFloor  = SkFixedFloorToFixed(currE->fUpperY);
            SkFixed lowerCeil   = SkFixedCeilToFixed(currE->fLowerY);
            int     iy          = SkFixedFloorToInt(upperFloor);

            if (lowerCeil <= upperFloor + SK_Fixed1) { // only one row is affected by the currE
                SkFixed rowHeight = currE->fLowerY - currE->fUpperY;
                SkFixed nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_coverage_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
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
            if (SkIntToFixed(iy) < currE->fLowerY) {
                rowHeight = currE->fLowerY - SkIntToFixed(iy);
                nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_coverage_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
            }
        // update returns true when we're done with it; hence continue if not done.
        } while (!currE->update(currE->fLowerY));
    }
}

void SkScan::DAAFillPath(const SkPath& path, const SkRegion& origClip, SkBlitter* blitter,
                         bool forceRLE) {

    FillPathFunc fillPathFunc = [](const SkPath& path, SkBlitter* blitter, bool isInverse,
            const SkIRect& ir, const SkRegion* clipRgn, const SkIRect* clipRect, bool forceRLE){
        bool isEvenOdd  = path.getFillType() & 1;
        bool isConvex   = path.isConvex();
        bool skipRect   = isConvex && !isInverse;

        const SkIRect& clipBounds = clipRgn->getBounds();
        SkIRect clippedIR = ir;
        clippedIR.intersect(clipBounds);

        // Only blitter->blitXXX need to be done in order in the threaded backend.
        // Everything before can be done out of order in the threaded backend.
        if (!forceRLE && !isInverse && SkCoverageDeltaMask::Suitable(clippedIR)) {
            SkCoverageDeltaMask deltaMask(clippedIR);
            gen_alpha_deltas(path, *clipRgn, deltaMask, blitter, skipRect, clipRect == nullptr);
            deltaMask.convertCoverageToAlpha(isEvenOdd, isInverse, isConvex);
            blitter->blitMask(deltaMask.prepareSkMask(), clippedIR);
        } else {
            SkCoverageDeltaAllocator alloc;
            SkCoverageDeltaList deltaList(&alloc, clippedIR.fTop, clippedIR.fBottom, forceRLE);
            gen_alpha_deltas(path, *clipRgn, deltaList, blitter, skipRect, clipRect == nullptr);
            blitter->blitCoverageDeltas(&deltaList, clipBounds, isEvenOdd, isInverse, isConvex);
        }
    };

    // For threaded backend with out-of-order init-once (and therefore out-of-order do_fill_path),
    // we probably have to take care of the blitRegion, sk_blit_above, sk_blit_below in do_fill_path
    // to maintain the draw order. If we do that, be caureful that blitRect may throw exception is
    // the rect is empty.
    do_fill_path(path, origClip, blitter, forceRLE, 2, std::move(fillPathFunc));
}
