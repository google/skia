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

// We will export SkAlphaDelta, SkAlphaDeltaCollection, SkDeltaBlitter in a future header file
// so other Skia components (e.g., threaded backend) can handle them.

// Some thoughts on the future of SkAlphaDelta:
//   1. fX and fY may have other types such as uint_16. Maybe we could template it.
//   2. Maybe we don't need fY because it can be inferred if we bucket the deltas in y
//      with a granularity of 1.
//   3. In the future, maybe we can also add fZ that indicates the draw number.
//      It can ensure the correct draw order and find us the blitter to blit the alpha.
//      That may allow us the maximum concurrency?
struct SkAlphaDelta {
    int     fX;
    SkFixed fDelta; // the amount that the alpha changed
};

using Allocator = SkSTArenaAlloc<2048>;

class SkAlphaDeltaCollection {
public:
    static constexpr int INIT_ROW_SIZE = 8;

    SkAlphaDeltaCollection(Allocator* alloc, int top, int bottom) {
        fAlloc = alloc;
        fTop = top;
        fBottom = bottom;

        fCounts = fAlloc->makeArrayDefault<int>((bottom - top) * 2);
        fMaxCounts = fCounts + bottom - top;
        memset(fCounts, 0, sizeof(int) * (bottom - top));
        fCounts -= top; // so we can directly use fCounts[y] instead of fCounts[y - fTop]
        fMaxCounts -= top;
        for(int y = top; y < bottom; ++y) {
            fMaxCounts[y] = INIT_ROW_SIZE;
        }

        fRows = fAlloc->makeArrayDefault<SkAlphaDelta*>(bottom - top) - top;
        fRows[top] = fAlloc->makeArrayDefault<SkAlphaDelta>(INIT_ROW_SIZE * (bottom - top));
        for(int y = top + 1; y < bottom; ++y) {
            fRows[y] = fRows[y - 1] + INIT_ROW_SIZE;
        }
    }

    inline void push_back(int y, const SkAlphaDelta& delta) {
        this->checkY(y);
        if (fCounts[y] == fMaxCounts[y]) {
            fMaxCounts[y] *= 2;
            SkAlphaDelta* newRow = fAlloc->makeArrayDefault<SkAlphaDelta>(fMaxCounts[y]);
            memcpy(newRow, fRows[y], sizeof(SkAlphaDelta) * fCounts[y]);
            fRows[y] = newRow;
        }
        SkASSERT(fCounts[y] < fMaxCounts[y]);

        fRows[y][fCounts[y]++] = delta;
        for(int i = fCounts[y] - 1; i > 0 && fRows[y][i - 1].fX > fRows[y][i].fX; --i) {
            SkTSwap(fRows[y][i], fRows[y][i - 1]);
        }
    }

    inline int getCount(int y) const {
        this->checkY(y);
        return fCounts[y];
    }

    inline const SkAlphaDelta& getDelta(int y, int i) const {
        this->checkY(y);
        SkASSERT(i < fCounts[y]);
        return fRows[y][i];
    }

    inline int top() const { return fTop; }
    inline int bottom() const { return fBottom; }

private:
    Allocator*      fAlloc;
    SkAlphaDelta**  fRows;
    int*            fCounts;
    int*            fMaxCounts;
    int             fTop;
    int             fBottom;

    inline void checkY(int y) const {
        SkASSERT(y >= fTop && y < fBottom);
    }
};

// convertLastAlpha will be much faster by using template
template<bool isEvenOdd, bool isInverse, bool useMask>
class SkDeltaBlitter {
public:
    SkDeltaBlitter(const SkIRect& bounds, SkBlitter* blitter) {
        fBounds = bounds;
        fBlitter = blitter;

        int runsSize = sizeof(int16_t) * (bounds.width() + 1);
        int alphasSize = sizeof(SkAlpha) * (bounds.width() + 1);
        fAlphas = static_cast<SkAlpha*>(blitter->allocBlitMemory(runsSize + alphasSize));
        fRuns = (int16_t*)(fAlphas + alphasSize);

        fRuns[0] = 0;
        int rightIndex = bounds.fRight - bounds.fLeft;
        fRuns[rightIndex] = 0; // always stop at the right bound
        fAlphas[0] = 0;
        fAlphas[rightIndex] = 0;
        fLastAlpha = 0;
        fLastY = bounds.fTop;
        fLastX = bounds.fLeft;

        fMask.fImage = fAlphas;
        fMask.fBounds = SkIRect::MakeXYWH(0, 0, bounds.fRight - bounds.fLeft, 1);
        fMask.fRowBytes = bounds.fRight - bounds.fLeft;
        fMask.fFormat = SkMask::kA8_Format;
    }

    ~SkDeltaBlitter() {
        this->flush();
    }

    // This function must be called in delta's x order. And the y must be the same as the fLastY
    inline void addDeltaInRow(int y, const SkAlphaDelta& delta) {
        // The caller should check the range so we don't have to do that at a cost of performance.
        SkASSERT(delta.fX >= fBounds.fLeft && delta.fX < fBounds.fRight &&
                y >= fBounds.fTop && y < fBounds.fBottom);
        SkASSERT(y == fLastY && delta.fX >= fLastX);
        if (fLastX != delta.fX) {
            int indexLastX = fLastX - fBounds.fLeft;
            if (useMask) {
                memset(fAlphas + indexLastX, this->convertLastAlpha(), delta.fX - fLastX);
            } else {
                fRuns[indexLastX] = delta.fX - fLastX;
                fAlphas[indexLastX] = this->convertLastAlpha();
            }
            fLastX = delta.fX;
        }

        fLastAlpha += delta.fDelta;
    }

    void flush() {
        int indexLastX = fLastX - fBounds.fLeft;
        fAlphas[indexLastX] = this->convertLastAlpha();
        if (useMask) {
            memset(fAlphas + indexLastX, fAlphas[indexLastX], fBounds.fRight - fLastX);
        } else {
            if (fAlphas[indexLastX] > 0) {
                // fRuns[fBounds.fRight - fBounds.fLeft] is always 0 so we don't have to set it here.
                fRuns[indexLastX] = fBounds.fRight - fLastX;
            } else {
                fRuns[indexLastX] = 0;
            }
        }
        // TODO shall we skip the leftmost run with 0 alpha (if blitter doesn't do that)?
        if (useMask) {
            fMask.fBounds = SkIRect::MakeXYWH(fBounds.fLeft, fLastY, fBounds.width(), 1);
            fBlitter->blitMask(fMask, fMask.fBounds);
        } else {
            if (fRuns[0]) {
                fBlitter->blitAntiH(fBounds.fLeft, fLastY, fAlphas, fRuns);
            }
        }
    }

    void initRow(int y) {
        this->flush();
        fLastY = y;
        fLastX = fBounds.fLeft;
        fRuns[0] = 0;
        fAlphas[0] = 0;
        fLastAlpha = 0;
    }

private:
    SkBlitter*  fBlitter;
    SkIRect     fBounds;
    int         fLastY;
    int         fLastX;
    SkFixed     fLastAlpha;
    SkAlpha*    fAlphas;
    int16_t*    fRuns;
    SkMask      fMask;

    inline SkAlpha convertLastAlpha() {
        SkAlpha result;
        if (isEvenOdd) {
            SkFixed mod17 = fLastAlpha & 0x1ffff;
            SkFixed mod16 = fLastAlpha & 0xffff;
            result = SkTPin(((mod16 << 1) - mod17) >> 8, 0, 255);
        } else {
            result = SkTPin(SkAbs32(fLastAlpha) >> 8, 0, 255);
        }
        return isInverse ? 255 - result : result;
    }
};

///////////////////////////////////////////////////////////////////////////////

// 2 versions of templates functions for less runtime branches and more speedup
template<bool isPartial>
static inline void add_alpha_delta_segment(int y, SkFixed rowHeight, const SkAnalyticEdge* edge,
        SkFixed nextX, SkAlphaDeltaCollection* deltas) {
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
        deltas->push_back(y, {L, alpha * sign});
        deltas->push_back(y, {L + 1, (rowHeight - alpha) * sign});
    } else if (len == 2) {
        SkFixed middle = SkIntToFixed(L + 1);
        SkFixed x1 = middle - l;
        SkFixed x2 = r - middle;
        SkFixed alpha1 = partialTriangleToAlpha(x1, edge->fDY);
        SkFixed alpha2 = rowHeight - partialTriangleToAlpha(x2, edge->fDY); // rowHeight=fullAlpha
        deltas->push_back(y, {L, alpha1 * sign});
        deltas->push_back(y, {L + 1, (alpha2 - alpha1) * sign});
        deltas->push_back(y, {L + 2, (rowHeight - alpha2) * sign});
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
        deltas->push_back(y, {L, alphas[0] * sign});
        for(int i = 1; i < len; ++i) {
            deltas->push_back(y, {L + i, (alphas[i] - alphas[i - 1]) * sign});
        }
        deltas->push_back(y, {L, (rowHeight - alphas[len - 1]) * sign});

        if (len > kQuickLen) {
            delete [] alphas;
        }
    }
}

static bool operator<(const SkAnalyticEdge& a, const SkAnalyticEdge& b) {
    return a.fUpperX < b.fUpperX || (a.fUpperX == b.fUpperX && a.fDX < b.fDX);
}

static void gen_sorted_alpha_deltas(
        const SkPath& path, const SkRegion& origClip, SkAlphaDeltaCollection& result) {
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

    SkTQSort(list, list + count - 1);

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

template<bool isEvenOdd, bool isInverse>
void blit_sorted_alpha_deltas(const SkAlphaDeltaCollection& deltas, const SkIRect& bounds,
        SkBlitter* blitter, bool forceRLE) {
    SkDeltaBlitter<isEvenOdd, isInverse, false> deltaBlitter(bounds, blitter);

    if (isInverse) {
        for(int y = bounds.fTop; y < deltas.top(); ++y) {
            deltaBlitter.initRow(y);
        }
    }

    for(int y = deltas.top(); y < deltas.bottom(); ++y) {
        deltaBlitter.initRow(y);
        int i = 0;
        for(; i < deltas.getCount(y) && deltas.getDelta(y, i).fX < bounds.fLeft; ++i);
        for(; i < deltas.getCount(y) && deltas.getDelta(y, i).fX < bounds.fRight; ++i) {
            deltaBlitter.addDeltaInRow(y, deltas.getDelta(y, i));
        }
    }

    if (isInverse) {
        for(int y = deltas.bottom(); y < bounds.fBottom; ++y) {
            deltaBlitter.initRow(y);
        }
    }
}

void SkScan::DAAFillPath(const SkPath& path, const SkRegion& origClip, SkBlitter* blitter,
                         bool forceRLE) {
    // TODO Check if the clipped path overflows SkFixed. If true, hand it to non-AA FillPath.

    // This step can be done out of order in the threaded backend
    const SkIRect& clipBounds = origClip.getBounds();
    SkIRect pathIR;
    path.getBounds().roundOut(&pathIR);
    SkIRect clippedIR = pathIR;
    clippedIR.intersect(clipBounds);
    Allocator alloc;
    SkAlphaDeltaCollection sortedAlphaDeltas(&alloc, clippedIR.fTop, clippedIR.fBottom);
    gen_sorted_alpha_deltas(path, origClip, sortedAlphaDeltas);

    // This step must be done in order in the threaded backend
    if (path.getFillType() & 1) { // isEvenOdd?
        if (path.isInverseFillType()) {
            blit_sorted_alpha_deltas<true, true>(sortedAlphaDeltas, clipBounds, blitter, forceRLE);
        } else {
            blit_sorted_alpha_deltas<true, false>(sortedAlphaDeltas, clippedIR, blitter, forceRLE);
        }
    } else {
        if (path.isInverseFillType()) {
            blit_sorted_alpha_deltas<false, true>(sortedAlphaDeltas, clipBounds, blitter, forceRLE);
        } else {
            blit_sorted_alpha_deltas<false, false>(sortedAlphaDeltas, clippedIR, blitter, forceRLE);
        }
    }
}
