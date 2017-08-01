/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCoverageDelta.h"

SkCoverageDeltaList::SkCoverageDeltaList(SkCoverageDeltaAllocator* alloc, const SkIRect& bounds,
                                         bool forceRLE) {
    fAlloc              = alloc;
    fBounds             = bounds.makeOutset(PADDING, 0);
    fForceRLE           = forceRLE;
    fCount              = 0;
    fMaxCount           = RESERVED_STORAGE;
    fDeltas             = fReservedStorage;
    fRows               = nullptr; // this will be generated in sort method

    if (fBounds.height() + fBounds.width() < RESERVED_HEIGHT + RESERVED_WIDTH) {
        fCountByX = fReservedCounts;
    } else {
        fCountByX = alloc->makeArrayDefault<int>(fBounds.width() + fBounds.height());
    }
    memset(fCountByX, 0, sizeof(int) * (fBounds.height() + fBounds.width()));
    fCountByY = fCountByX + fBounds.width();
    fCountByX -= fBounds.fLeft; // so we can directly use fCountByX[x]
    fCountByY -= fBounds.fTop;  // so we can directly use fCountByY[y]

    // Init the anti-rect to be empty
    fAntiRect.fY        = fBounds.fBottom;
    fAntiRect.fHeight   = 0;
}

void SkCoverageDeltaList::sort() {
    SkCoverageDelta* sortStorage =
            fCount <= RESERVED_STORAGE / 2 ? fDeltas + fCount
                                           : fAlloc->makeArrayDefault<SkCoverageDelta>(fCount);

    int* sortCounts = fReservedCounts + fBounds.height() + fBounds.width();
    if (fBounds.height() + fBounds.width() > (RESERVED_HEIGHT + RESERVED_WIDTH) / 2) {
        sortCounts = fAlloc->makeArrayDefault<int>(fBounds.height() + fBounds.width());
    }

    fRows = fReservedRows;
    if (SkTMax(fBounds.height(), fBounds.width()) > RESERVED_HEIGHT) {
        fRows = (SkCoverageDelta**)fAlloc->makeArrayDefault<SkCoverageDelta**>(
            SkTMax(fBounds.height(), fBounds.width()));
    }

    // first sort by x
    memset(sortCounts, 0, sizeof(int) * fBounds.width());
    int* countX = sortCounts - fBounds.fLeft;
    fRows[0] = sortStorage;
    fRows -= fBounds.fLeft;
    for(int x = fBounds.fLeft + 1; x < fBounds.fRight; ++x) {
        fRows[x] = fRows[x - 1] + fCountByX[x - 1];
    }
    for(int i = 0; i < fCount; ++i) {
        const SkCoverageDelta& delta = fDeltas[i];
        fRows[delta.fX][countX[delta.fX]++] = delta;
    }
    memcpy(fDeltas, sortStorage, sizeof(SkCoverageDelta) * fCount);
    fRows += fBounds.fLeft; // restore

    // then sort by y
    memset(sortCounts, 0, sizeof(int) * fBounds.height());
    int* countY = sortCounts - fBounds.fTop;
    fRows[0] = sortStorage;
    fRows -= fBounds.fTop; // from now on, we can use fRows[y]
    for(int y = fBounds.fTop + 1; y < fBounds.fBottom; ++ y) {
        fRows[y] = fRows[y - 1] + fCountByY[y - 1];
    }
    for(int i = 0; i < fCount; ++i) {
        const SkCoverageDelta& delta = fDeltas[i];
        fRows[delta.fY][countY[delta.fY]++] = delta;
    }
    memcpy(fDeltas, sortStorage, sizeof(SkCoverageDelta) * fCount);
}

int SkCoverageDeltaMask::ExpandWidth(int width) {
    int result = width + PADDING * 2;
    return result + (SIMD_WIDTH - result % SIMD_WIDTH) % SIMD_WIDTH;
}

bool SkCoverageDeltaMask::CanHandle(const SkIRect& bounds) {
    // Expand width so we don't have to worry about the boundary
    return ExpandWidth(bounds.width()) * bounds.height() + PADDING * 2 < MAX_MASK_SIZE;
}

bool SkCoverageDeltaMask::Suitable(const SkIRect& bounds) {
    return bounds.width() <= SUITABLE_WIDTH && CanHandle(bounds);
}

SkCoverageDeltaMask::SkCoverageDeltaMask(const SkIRect& bounds) : fBounds(bounds) {
    SkASSERT(CanHandle(bounds));

    // Init the anti-rect to be empty
    fAntiRect.fY        = fBounds.fBottom;
    fAntiRect.fHeight   = 0;

    fExpandedWidth      = ExpandWidth(fBounds.width());

    // Add PADDING columns so we may access fDeltas[index(-PADDING, 0)]
    // Minus index(fBounds.fLeft, fBounds.fTop) so we can directly access fDeltas[index(x, y)]
    fDeltas             = fDeltaStorage + PADDING - this->index(fBounds.fLeft, fBounds.fTop);

    memset(fDeltaStorage, 0, (fExpandedWidth * bounds.height() + PADDING * 2) * sizeof(SkFixed));;
}

void SkCoverageDeltaMask::convertCoverageToAlpha(bool isEvenOdd, bool isInverse, bool isConvex) {
    SkFixed* deltaRow = &this->delta(fBounds.fLeft, fBounds.fTop);
    SkAlpha* maskRow = fMask;
    for(int iy = 0; iy < fBounds.height(); ++iy) {
        // If we're inside fAntiRect, blit it to the mask and advance to its bottom
        if (fAntiRect.fHeight && iy == fAntiRect.fY - fBounds.fTop) {
            // Blit the mask
            int L = fAntiRect.fX - fBounds.fLeft;
            for(int i = 0; i < fAntiRect.fHeight; ++i) {
                SkAlpha* tMask = maskRow + L;
                if (fAntiRect.fLeftAlpha) {
                    tMask[0] = fAntiRect.fLeftAlpha;
                }
                memset(tMask + 1, 0xff, fAntiRect.fWidth);
                if (fAntiRect.fRightAlpha) {
                    tMask[fAntiRect.fWidth + 1] = fAntiRect.fRightAlpha;
                }
                maskRow += fBounds.width();
            }

            // Advance to the bottom (maskRow is already advanced to the bottom).
            deltaRow    += fExpandedWidth * fAntiRect.fHeight;
            iy          += fAntiRect.fHeight - 1; // -1 because we'll ++iy after continue
            continue;
        }

        // Otherwise, cumulate deltas into coverages, and convert them into alphas
        SkFixed c[SIMD_WIDTH] = {0}; // prepare SIMD_WIDTH coverages at a time
        for(int ix = 0; ix < fExpandedWidth; ix += SIMD_WIDTH) {
            // Future todo: is it faster to process SIMD_WIDTH rows at a time so we can use SIMD
            // for coverage accumulation?

            // Cumulate deltas to get SIMD_WIDTH new coverages
            c[0] = c[SIMD_WIDTH - 1] + deltaRow[ix];
            for(int j = 1; j < SIMD_WIDTH; ++j) {
                c[j] = c[j - 1] + deltaRow[ix + j];
            }

            // My SIMD CoverageToAlpha seems to be only faster with SSSE3.
            // (On linux, even with -mavx2, my SIMD still seems to be slow...)
            // Even with only SSSE2, it's still faster to do SIMD_WIDTH non-SIMD computations at one
            // time (i.e., SIMD_WIDTH = 8 is faster than SIMD_WIDTH = 1 even if SK_CPU_SSE_LEVEL is
            // less than SK_CPU_SSE_LEVEL_SSSE3). Maybe the compiler is doing some SIMD by itself.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
            using SkNi = SkNx<SIMD_WIDTH, int>;

            SkNi cn = SkNi::Load(c);
            SkNi an = isConvex ? ConvexCoverageToAlpha(cn, isInverse)
                               : CoverageToAlpha(cn, isEvenOdd, isInverse);
            SkNx_cast<SkAlpha>(an).store(maskRow + ix);
#else
            for(int j = 0; j < SIMD_WIDTH; ++j) {
                maskRow[ix + j] = isConvex ? ConvexCoverageToAlpha(c[j], isInverse)
                                           : CoverageToAlpha(c[j], isEvenOdd, isInverse);
            }
#endif
        }

        // Finally, advance to the next row
        deltaRow    += fExpandedWidth;
        maskRow     += fBounds.width();
    }
}
