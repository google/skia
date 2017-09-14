/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCoverageDelta.h"

SkCoverageDeltaList::SkCoverageDeltaList(SkArenaAlloc* alloc, int top, int bottom, bool forceRLE) {
    fAlloc              = alloc;
    fTop                = top;
    fBottom             = bottom;
    fForceRLE           = forceRLE;

    // Init the anti-rect to be empty
    fAntiRect.fY        = bottom;
    fAntiRect.fHeight   = 0;

    fSorted     = fAlloc->makeArrayDefault<bool>(bottom - top);
    fCounts     = fAlloc->makeArrayDefault<int>((bottom - top) * 2);
    fMaxCounts  = fCounts + bottom - top;
    fRows       = fAlloc->makeArrayDefault<SkCoverageDelta*>(bottom - top) - top;
    fRows[top]  = fAlloc->makeArrayDefault<SkCoverageDelta>(INIT_ROW_SIZE * (bottom - top));

    memset(fSorted, true, bottom - top);
    memset(fCounts, 0, sizeof(int) * (bottom - top));

    // Minus top so we can directly use fCounts[y] instead of fCounts[y - fTop].
    // Same for fMaxCounts, fRows, and fSorted.
    fSorted    -= top;
    fCounts    -= top;
    fMaxCounts -= top;

    for(int y = top; y < bottom; ++y) {
        fMaxCounts[y] = INIT_ROW_SIZE;
    }
    for(int y = top + 1; y < bottom; ++y) {
        fRows[y] = fRows[y - 1] + INIT_ROW_SIZE;
    }
}

SkCoverageDeltaList::SkCoverageDeltaList(SkArenaAlloc* alloc, SkCoverageDeltaList* other) {
    fAlloc = alloc;
    fTop = other->fTop;
    fBottom = other->fBottom;
    fForceRLE = other->fForceRLE;
    fAntiRect = other->fAntiRect;

    int h = fBottom - fTop;

    fSorted     = fAlloc->makeArrayDefault<bool>(h);
    fCounts     = fAlloc->makeArrayDefault<int>(h * 2);
    fMaxCounts  = fCounts + h;
    fRows       = fAlloc->makeArrayDefault<SkCoverageDelta*>(h) - fTop;

    // Minus top so we can directly use fCounts[y] instead of fCounts[y - fTop].
    // Same for fMaxCounts, fRows, and fSorted.
    fSorted    -= fTop;
    fCounts    -= fTop;
    fMaxCounts -= fTop;

    memcpy(fSorted + fTop, other->fSorted + fTop, sizeof(bool) * h);
    memcpy(fCounts + fTop, other->fCounts + fTop, sizeof(int) * h);
    memcpy(fMaxCounts + fTop, other->fCounts + fTop, sizeof(int) * h); // maxCount = count

    int totalCount = 0;
    for(int i = fTop; i < fBottom; ++i) {
        totalCount += fCounts[i];
    }
    fRows[fTop]  = fAlloc->makeArrayDefault<SkCoverageDelta>(totalCount);
    for(int y = fTop; y < fBottom; ++y) {
        memcpy(fRows[y], other->fRows[y], sizeof(SkCoverageDelta) * fCounts[y]);
        if (y < fBottom - 1) {
            fRows[y + 1] = fRows[y] + fCounts[y];
        }
    }
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

SkCoverageDeltaMask::SkCoverageDeltaMask(SkArenaAlloc* alloc, const SkIRect& bounds) {
    SkASSERT(CanHandle(bounds));

    fBounds             = bounds;

    // Init the anti-rect to be empty
    fAntiRect.fY        = fBounds.fBottom;
    fAntiRect.fHeight   = 0;

    fExpandedWidth      = ExpandWidth(fBounds.width());

    int size            = fExpandedWidth * bounds.height() + PADDING * 2;
    fDeltaStorage       = alloc->makeArray<SkFixed>(size);
    fMask               = alloc->makeArrayDefault<SkAlpha>(size);

    // Add PADDING columns so we may access fDeltas[index(-PADDING, 0)]
    // Minus index(fBounds.fLeft, fBounds.fTop) so we can directly access fDeltas[index(x, y)]
    fDeltas             = fDeltaStorage + PADDING - this->index(fBounds.fLeft, fBounds.fTop);
}

// TODO As this function is so performance-critical (and we're thinking so much about SIMD), use
// SkOpts framework to compile multiple versions of this function so we can choose the best one
// available at runtime.
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

            using SkNi = SkNx<SIMD_WIDTH, int>;
            SkNi cn = SkNi::Load(c);
            SkNi an = isConvex ? ConvexCoverageToAlpha(cn, isInverse)
                               : CoverageToAlpha(cn, isEvenOdd, isInverse);
            SkNx_cast<SkAlpha>(an).store(maskRow + ix);
        }

        // Finally, advance to the next row
        deltaRow    += fExpandedWidth;
        maskRow     += fBounds.width();
    }
}

void SkCoverageRecord::addMask(const SkMask& mask) {
    // The mask's memory may be temporary so we have to copy it.
    // We'll eventually use blitCoverageDeltas(SkCoverageDeltaMask) to avoid copying.
    fMasks.push_back(mask);
    SkMask& newMask = fMasks[fMasks.size() - 1];
    newMask.fImage = fAlloc->makeArrayDefault<SkAlpha>(mask.fRowBytes * mask.fBounds.height());
    memcpy(newMask.fImage, mask.fImage, mask.fRowBytes * mask.fBounds.height());
}

void SkCoverageRecord::addList(SkCoverageDeltaList* deltas) {
    // TODO instead copying the whole list, maybe we want to process it and only store SkAlphas
    // and runs.
    fLists.emplace_back(fAlloc, deltas);
}
