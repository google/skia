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

#include <algorithm>

///////////////////////////////////////////////////////////////////////////////

/*

DAA stands for Delta-based Anti-Aliasing.

This is an improved version of our analytic AA algorithm (in SkScan_AAAPath.cpp)
which first scan convert paths into alpha deltas (this step can happen out of order,
and we don't seem to be needed to worry about the intersection, clamping, etc.),
and then use a single blitter run to convert all those deltas into the final mask/pixels.

Before we go to the final blitter run, we'll use SkFixed for all alpha values so we
don't ever have to worry about under/overflow :)

*/

///////////////////////////////////////////////////////////////////////////////

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

// Here we always send in l < SK_Fixed1, and the first alpha we want to compute is alphas[0]
// We also require that the length (SkFixedCeilToInt(r)) is at least 3. For shoter length,
// don't use this method, and just compute the alpha directly (which is simpler and faster).
static inline void computeAlphaAboveLine(
        SkFixed* alphas, SkFixed l, SkFixed r, SkFixed dY, SkFixed fullAlpha) {
    SkASSERT(l <= r);
    SkASSERT(l >> 16 == 0);
    int R = SkFixedCeilToInt(r);
    SkASSERT(R >= 2);
    SkFixed first = SK_Fixed1 - l; // horizontal edge length of the left-most triangle
    SkFixed last = r - ((R - 1) << 16); // horizontal edge length of the right-most triangle
    SkFixed firstH = SkFixedMul(first, dY); // vertical edge of the left-most triangle
    alphas[0] = SkFixedMul(first, firstH) >> 1; // triangle alpha
    alphas[1] = firstH + (dY >> 1); // rectangle plus triangle
    for (int i = 2; i < R - 1; ++i) {
        alphas[i] = alphas[i - 1] + dY;
    }
    alphas[R - 1] = fullAlpha - partialTriangleToAlpha(last, dY);
}

///////////////////////////////////////////////////////////////////////////////

// return true if we're done with this edge
static bool update_edge(SkAnalyticEdge* edge, SkFixed last_y) {
    if (last_y >= edge->fLowerY) {
        if (edge->fCurveCount < 0) {
            if (static_cast<SkAnalyticCubicEdge*>(edge)->updateCubic()) {
                return false;
            }
        } else if (edge->fCurveCount > 0) {
            if (static_cast<SkAnalyticQuadraticEdge*>(edge)->updateQuadratic()) {
                return false;
            }
        }
        return true;
    }
    SkASSERT(false);
    return false;
}

///////////////////////////////////////////////////////////////////////////////

// 2 versions of templates functions for less runtime branches and more speedup
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
        SkFixed fixedR = SkIntToFixed(R);
        SkFixed alpha = trapezoidToAlpha(fixedR - l, fixedR - r);
        if (isPartial) {
            alpha = getPartialAlpha(alpha, rowHeight);
        }
        deltas->addDelta(L, y, alpha * sign);
        deltas->addDelta(L + 1, y, (rowHeight - alpha) * sign);
    } else if (len == 2) {
        SkFixed middle = SkIntToFixed(L + 1);
        SkFixed x1 = middle - l;
        SkFixed x2 = r - middle;
        SkFixed alpha1 = partialTriangleToAlpha(x1, edge->fDY);
        SkFixed alpha2 = rowHeight - partialTriangleToAlpha(x2, edge->fDY); // rowHeight=fullAlpha
        deltas->addDelta(L, y, alpha1 * sign);
        deltas->addDelta(L + 1, y, (alpha2 - alpha1) * sign);
        deltas->addDelta(L + 2, y, (rowHeight - alpha2) * sign);
    } else {
        const int kQuickLen = 128;
        // This is faster than SkAutoSMalloc<1024>
        char quickMemory[kQuickLen * sizeof(SkFixed)];
        SkFixed* alphas;

        if (len <= kQuickLen) {
            alphas = (SkFixed*)quickMemory;
        } else {
            alphas = new SkFixed[len];
        }

        SkFixed fixedL = SkIntToFixed(L);
        SkASSERT(SkFixedCeilToInt(r - fixedL) == len);
        computeAlphaAboveLine(alphas, l - fixedL, r - fixedL, edge->fDY, rowHeight);
        deltas->addDelta(L, y, alphas[0] * sign);
        for(int i = 1; i < len; ++i) {
            deltas->addDelta(L + i, y, (alphas[i] - alphas[i - 1]) * sign);
        }
        deltas->addDelta(L, y, (rowHeight - alphas[len - 1]) * sign);

        if (len > kQuickLen) {
            delete [] alphas;
        }
    }
}

class LessThan {
public:
    bool operator()(const SkAnalyticEdge* a, const SkAnalyticEdge* b) {
        return a->fUpperX < b->fUpperX || (a->fUpperX == b->fUpperX && a->fDX < b->fDX);
    }
};


template<class Deltas>
static void gen_alpha_deltas(
        const SkPath& path, const SkRegion& origClip, Deltas& result) {
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

    LessThan lessThan;
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

            // first row
            SkFixed rowHeight = upperFloor + SK_Fixed1 - currE->fUpperY;
            SkFixed nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
            add_alpha_delta_segment<true>(iy, rowHeight, currE, nextX, &result);

            while (true) {
                iy++;
                SkFixed y = SkIntToFixed(iy);
                currE->fX = nextX;
                nextX += currE->fDX;

                if (y + SK_Fixed1 >= lowerCeil) {
                    break;
                }

                add_alpha_delta_segment<false>(iy, SK_Fixed1, currE, nextX, &result);
            }

            // last row
            rowHeight = currE->fLowerY - SkIntToFixed(iy);
            nextX = currE->fX + SkFixedMul(currE->fDX, rowHeight);
            add_alpha_delta_segment<true>(iy, rowHeight, currE, nextX, &result);
        // update_edge returns true when we're done with it
        } while (!update_edge(currE, currE->fLowerY));
    }
}

void blit_alpha_deltas(SkCoverageDeltaList& deltas, const SkIRect& bounds,
        SkBlitter* blitter, bool forceRLE, bool isEvenOdd, bool isInverse, bool isConvex) {
    blitter->blitCoverageDeltas(&deltas, bounds, isEvenOdd, isInverse, isConvex);
}

void SkScan::DAAFillPath(const SkPath& path, const SkRegion& origClip, SkBlitter* blitter,
                         bool forceRLE) {
    if (path.isRect(nullptr)) {
        // We really couldn't go faster than AAA when it's a rect. Therefore fall back.
        // Actually, it will be much slower if we generate deltas for a rect rather than directly
        // calling blitRect or blitAntiRect.
        AAAFillPath(path, origClip, blitter, forceRLE);
        return;
    }

    // TODO Check if the clipped path overflows SkFixed. If true, hand it to non-AA FillPath.

    bool isEvenOdd = path.getFillType() & 1;
    bool isInverse = path.isInverseFillType();
    bool isConvex = path.isConvex();
    const SkIRect& clipBounds = origClip.getBounds();
    SkIRect pathIR;
    path.getBounds().roundOut(&pathIR);
    SkIRect clippedIR = pathIR;
    clippedIR.intersect(clipBounds);

    if (SkCoverageDeltaMask::Suitable(clippedIR)) {
        SkCoverageDeltaMask deltaMask(clippedIR);
        gen_alpha_deltas(path, origClip, deltaMask);

        // This step must be done in order in the threaded backend
        // Everything before can be done out of order in the threaded backend
        blitter->blitCoverageDeltas(&deltaMask, clipBounds, isEvenOdd, isInverse, isConvex);
    } else {
        SkCoverageDeltaAllocator alloc;
        SkCoverageDeltaList deltaList(&alloc, clippedIR.fTop, clippedIR.fBottom);
        gen_alpha_deltas(path, origClip, deltaList);

        // This step must be done in order in the threaded backend
        // Everything before can be done out of order in the threaded backend
        blitter->blitCoverageDeltas(&deltaList, clipBounds, isEvenOdd, isInverse, isConvex);
    }
}
