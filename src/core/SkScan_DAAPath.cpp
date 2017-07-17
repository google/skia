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
static inline void add_alpha_delta_segment(int y, SkFixed rowHeight, const SkAnalyticEdge* edge,
        SkFixed nextX, Deltas* deltas) {
    SkASSERT(rowHeight <= SK_Fixed1 && rowHeight >= 0);

    // Let's see if multiplying sign is faster than multiplying edge->fWinding.
    // (Compiler should be able to optimize multiplication with 1/-1?)
    int sign = edge->fWinding == 1 ? 1 : -1;

    SkFixed l = SkTMin(edge->fX, nextX);
    SkFixed r = edge->fX + nextX - l;
    int     L = SkFixedFloorToInt(l);
    int     R = SkFixedCeilToInt(r);
    int     len = R - L;

    if (len <= 1) {
        SkFixed fixedR  = SkIntToFixed(R);
        SkFixed alpha   = trapezoidToAlpha(fixedR - l, fixedR - r);
        if (isPartial) {
            alpha = getPartialAlpha(alpha, rowHeight);
        }
        deltas->addDelta(L,     y,  alpha * sign);
        deltas->addDelta(L + 1, y,  (rowHeight - alpha) * sign);
    } else if (len == 2) {
        SkFixed middle  = SkIntToFixed(L + 1);
        SkFixed x1      = middle - l;
        SkFixed x2      = r - middle;
        SkFixed alpha1  = partialTriangleToAlpha(x1, edge->fDY);
        SkFixed alpha2  = rowHeight - partialTriangleToAlpha(x2, edge->fDY); // rowHeight=fullAlpha
        deltas->addDelta(L,     y,  alpha1 * sign);
        deltas->addDelta(L + 1, y,  (alpha2 - alpha1) * sign);
        deltas->addDelta(L + 2, y,  (rowHeight - alpha2) * sign);
    } else {
        // The following is analogous to computeAlphaAboveLine in SkScan_AAAPath.cpp
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
        int va = a->fUpperY;
        int vb = b->fUpperY;
        if (va == vb) {
            va = a->fUpperX;
            vb = b->fUpperX;
        }
        if (va == vb) {
            va = a->fDX;
            vb = b->fDX;
        }
        return va < vb;
    }
};

template<class Deltas>
static void gen_alpha_deltas(
        const SkPath& path, const SkRegion& origClip, Deltas& result,
        SkBlitter* blitter, bool isConvex, bool skipRect) {
    SkIRect ir;
    if (!safeRoundOut(path.getBounds(), &ir, SK_MaxS32)) {
        // Bounds can't fit in SkIRect; we'll return without drawing (empty delta collection)
        return;
    }

    const SkIRect& clipRect = origClip.getBounds();
    bool pathContainedInClip = clipRect.contains(ir);

    SkEdgeBuilder builder;
    int count = builder.build_edges(path, &clipRect, 0, pathContainedInClip, true);
    if (count == 0) {
        return;
    }
    SkAnalyticEdge** list = builder.analyticEdgeList();

    int rectTop = ir.fBottom;
    int rectBot = ir.fBottom;
    if (isConvex) {
        YXLessThan lessThan;
        SkTQSort(list, list + count - 1, lessThan);

        // this makes sure that the coverage is between 0 to SK_Fixed1
        if (list[0]->fWinding != 1) {
            for(int i = 0; i < count; ++i) {
                list[i]->fWinding *= -1;
            }
        }
    }

    // Try to find the rect part because blitAntiRect is so much faster than blitCoverageDeltas
    if (skipRect) {
        SkASSERT(isConvex); // we only skip when it's convex and we must have YX sorted edges
        for(int i = 0; i < count - 1; ++i) {
            SkAnalyticEdge* l = list[i];
            SkAnalyticEdge* r = list[i + 1];
            SkFixed xorUpperY = l->fUpperY ^ r->fUpperY;
            SkFixed xorLowerY = l->fLowerY ^ r->fLowerY;
            SkFixed orDX = l->fDX | r->fDX;
            if ((xorUpperY | xorLowerY | orDX) == 0) {
                rectTop = SkFixedCeilToInt(l->fUpperY);
                rectBot = SkFixedFloorToInt(l->fLowerY);
                if (rectBot > rectTop) {
                    int L = SkFixedCeilToInt(l->fUpperX);
                    int R = SkFixedFloorToInt(r->fUpperX);
                    if (L <= R) {
                        SkAlpha la = (SkIntToFixed(L) - l->fUpperX) >> 8;
                        SkAlpha ra = (r->fUpperX - SkIntToFixed(R)) >> 8;
                        result.setAntiRect(L - 1, rectTop, R - L, rectBot - rectTop, la, ra);
                    } else {
                        // too thin to use blitAntiRect; reset the rect region to be emtpy
                        rectTop = rectBot = ir.fBottom;
                    }
                }
                break;
            }
        }
    }

    XLessThan lessThan;
    SkTQSort(list, list + count - 1, lessThan);
#ifdef SK_DEBUG
    for(int i = 0; i < count - 1; ++i) {
        SkASSERT(!lessThan(list[i + 1], list[i]));
    }
#endif

    // Future todo: much of the following can be parallelized, and SIMDed.

    for(int edgeIndex = 0; edgeIndex < count; ++edgeIndex) {
        // Future todo: SkAnalyticEdge has several fields (e.g., fNext, fPrev, fRiteE...) that are
        // useless here. Remove them may bring performance improvement.
        SkAnalyticEdge* currE = list[edgeIndex];

        do {
            currE->fX =  currE->fUpperX;

            SkFixed upperFloor = SkFixedFloorToFixed(currE->fUpperY);
            SkFixed lowerCeil = SkFixedCeilToFixed(currE->fLowerY);
            int iy = SkFixedFloorToInt(upperFloor);

            if (lowerCeil <= upperFloor + SK_Fixed1) { // only one row is affected by the currE
                SkFixed rowHeight = currE->fLowerY - currE->fUpperY;
                SkFixed nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_alpha_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
                continue;
            }

            // first partial row
            SkFixed rowHeight = upperFloor + SK_Fixed1 - currE->fUpperY;
            SkFixed nextX;
            if (rowHeight != SK_Fixed1) {
                nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_alpha_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
            } else {
                iy--; // compensate the iy++ in the while loop
                nextX = currE->fX;
            }

            while (true) {
                iy++;
                SkFixed y = SkIntToFixed(iy);
                currE->fX = nextX;
                nextX += currE->fDX;

                if (y + SK_Fixed1 > currE->fLowerY) {
                    break;
                }

                if (iy >= rectTop && iy < rectBot) {
                    SkASSERT(currE->fDX == 0); // we must be on an edge with fDX = 0
                    iy = rectBot - 1; // skip the rect part where we already blitted
                    continue;
                }

                add_alpha_delta_segment<false>(iy, SK_Fixed1, currE, nextX, &result);
            }

            // last partial row
            if (SkIntToFixed(iy) < currE->fLowerY) {
                rowHeight = currE->fLowerY - SkIntToFixed(iy);
                nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
                add_alpha_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
            }
        // update returns true when we're done with it
        } while (!currE->update(currE->fLowerY));
    }
}

void SkScan::DAAFillPath(const SkPath& path, const SkRegion& origClip, SkBlitter* blitter,
                         bool forceRLE) {
    SkIRect pathIR;
    if (!safeRoundOut(path.getBounds(), &pathIR, SK_MaxS32 >> 2)) {
        // Bounds can't fit in SkIRect; we'll return without drawing
        return;
    }

    bool isEvenOdd = path.getFillType() & 1;
    bool isInverse = path.isInverseFillType();
    bool isConvex = path.isConvex();

    bool blitEverything = false;
    bool isEmpty        = false;
    if (pathIR.isEmpty()) {
        if (isInverse) {
            blitEverything = true;
        }
        isEmpty = true;
    }

    const SkIRect& clipBounds = origClip.getBounds();
    path.getBounds().roundOut(&pathIR);
    SkIRect clippedIR = pathIR;
    if (!clippedIR.intersect(clipBounds)) {
        if (isInverse) {
            blitEverything = true;
        }
        isEmpty = true;
    }

    if (blitEverything) {
        // For threaded backend with init once, we probably need to save the region
        // in our SkCoverageDeltaList/Mask and blit it later.
        blitter->blitRegion(origClip);
    }
    if (isEmpty) {
        return;
    }

    if (rect_overflows_short_shift(clippedIR, 2)) {
        SkScan::FillPath(path, origClip, blitter);
        return;
    }

    // We should NOT skip the rect in threaded backend to maintain the draw order.
    bool skipRect = isConvex && !isInverse;

    // If skipRect, we don't use mask because the majority of the path
    // has already been handled by blitAntiRect.
    if (SkCoverageDeltaMask::Suitable(clippedIR)) {
        SkCoverageDeltaMask deltaMask(clippedIR);
        gen_alpha_deltas(path, origClip, deltaMask, blitter, isConvex, skipRect);

        // This step must be done in order in the threaded backend
        // Everything before can be done out of order in the threaded backend
        blitter->blitCoverageDeltas(&deltaMask, clipBounds, isEvenOdd, isInverse, isConvex);
    } else {
        SkCoverageDeltaAllocator alloc;
        SkCoverageDeltaList deltaList(&alloc, clippedIR.fTop, clippedIR.fBottom);
        gen_alpha_deltas(path, origClip, deltaList, blitter, isConvex, skipRect);

        // This step must be done in order in the threaded backend
        // Everything before can be done out of order in the threaded backend
        blitter->blitCoverageDeltas(&deltaList, clipBounds, isEvenOdd, isInverse, isConvex);
    }
}
