/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCoverageDelta_DEFINED
#define SkCoverageDelta_DEFINED

#include "SkArenaAlloc.h"
#include "SkFixed.h"
#include "SkTSort.h"
#include "SkUtils.h"

struct SkCoverageDelta {
    int     fX;     // the y coordinate will be implied in SkCoverageDeltaList
    SkFixed fDelta; // the amount that the alpha changed

    // Sort according to fX
    bool operator<(const SkCoverageDelta& other) const {
        return fX < other.fX;
    }
};

// todo future: SIMD?
static SK_ALWAYS_INLINE SkAlpha CoverageToAlpha(SkFixed coverage, bool isEvenOdd, bool isInverse) {
    SkAlpha result;
    if (isEvenOdd) {
        SkFixed mod17 = coverage & 0x1ffff;
        SkFixed mod16 = coverage & 0xffff;
        result = SkTPin(((mod16 << 1) - mod17) >> 8, 0, 255);
    } else {
        result = SkTPin(SkAbs32(coverage) >> 8, 0, 255);
    }
    return isInverse ? 255 - result : result;
}

static SK_ALWAYS_INLINE Sk4i CoverageToAlpha(Sk4i coverage, bool isEvenOdd, bool isInverse) {
    Sk4i t0(0), t255(255);
    Sk4i result;
    if (isEvenOdd) {
        Sk4i mod17 = coverage & 0x1ffff;
        Sk4i mod16 = coverage & 0xffff;
        result = ((mod16 << 1) - mod17) >> 8;
    } else {
        result = coverage.abs() >> 8;;
    }
    result = Sk4i::Min(result, t255);
    result = Sk4i::Max(result, t0);
    return isInverse ? 255 - result : result;
}

// For convex paths (including inverse mode), the coverage is guaranteed to be
// between [0, SK_Fixed1] so we can ignore isEvenOdd and SkTPin.
static SK_ALWAYS_INLINE SkAlpha ConvexCoverageToAlpha(SkFixed coverage, bool isInverse) {
    SkASSERT(coverage >= -SK_Fixed1 && coverage <= SK_Fixed1);
    int result = SkAbs32(coverage) >> 8;
    result -= (result >> 8); // 256 to 255
    return isInverse ? 255 - result : result;
}

static SK_ALWAYS_INLINE Sk4i ConvexCoverageToAlpha(Sk4i coverage, bool isInverse) {
    // allTrue is not implemented
    // SkASSERT((coverage >= -SK_Fixed1).allTrue() && (coverage <= SK_Fixed1).allTrue());
    Sk4i result = coverage.abs() >> 8;
    result -= (result >> 8); // 256 to 255
    return isInverse ? 255 - result : result;
}

using SkCoverageDeltaAllocator = SkSTArenaAlloc<256>;

// A list of SkCoverageDelta with y from top() to bottom().
// For each row y, there are count(y) number of deltas.
// You can ask whether they are sorted or not by isSorted(y), and you can sort them by sort(y).
// Once sorted, getDelta(y, i) should return the i-th leftmost delta on row y.
class SkCoverageDeltaList {
public:
    // We can store 8 deltas per row (i.e., per y-scanline) initially
    static constexpr int INIT_ROW_SIZE = 8;

    SkCoverageDeltaList(SkCoverageDeltaAllocator* alloc, int top, int bottom) {
        fAlloc = alloc;
        fTop = top;
        fBottom = bottom;

        fCounts = fAlloc->makeArrayDefault<int>((bottom - top) * 2);
        fMaxCounts = fCounts + bottom - top;
        memset(fCounts, 0, sizeof(int) * (bottom - top));
        // Minus top so we can directly use fCounts[y] instead of fCounts[y - fTop].
        // Same for fMaxCounts, fRows, and fSorted.
        fCounts -= top;
        fMaxCounts -= top;
        for(int y = top; y < bottom; ++y) {
            fMaxCounts[y] = INIT_ROW_SIZE;
        }

        fRows = fAlloc->makeArrayDefault<SkCoverageDelta*>(bottom - top) - top;
        fRows[top] = fAlloc->makeArrayDefault<SkCoverageDelta>(INIT_ROW_SIZE * (bottom - top));
        for(int y = top + 1; y < bottom; ++y) {
            fRows[y] = fRows[y - 1] + INIT_ROW_SIZE;
        }

        fSorted = fAlloc->makeArrayDefault<bool>(bottom - top);
        memset(fSorted, true, bottom - top);
        fSorted -= top;
    }

    inline void push_back(int y, const SkCoverageDelta& delta) {
        this->checkY(y);
        if (fCounts[y] == fMaxCounts[y]) {
            fMaxCounts[y] *= 2;
            SkCoverageDelta* newRow = fAlloc->makeArrayDefault<SkCoverageDelta>(fMaxCounts[y]);
            memcpy(newRow, fRows[y], sizeof(SkCoverageDelta) * fCounts[y]);
            fRows[y] = newRow;
        }
        SkASSERT(fCounts[y] < fMaxCounts[y]);

        fRows[y][fCounts[y]++] = delta;
#ifdef INSERTION_SORT_WHILE_PUSH_BACK
        for(int i = fCounts[y] - 1; i > 0 && fRows[y][i - 1].fX > fRows[y][i].fX; --i) {
            SkTSwap(fRows[y][i], fRows[y][i - 1]);
        }
#else
        fSorted[y] = fSorted[y] && (fCounts[y] == 1 || delta.fX >= fRows[y][fCounts[y] - 2].fX);
#endif
    }

    inline int count(int y) const {
        this->checkY(y);
        return fCounts[y];
    }

    inline const SkCoverageDelta& getDelta(int y, int i) const {
        this->checkY(y);
        SkASSERT(i < fCounts[y]);
        return fRows[y][i];
    }

    inline int top() const { return fTop; }
    inline int bottom() const { return fBottom; }

    inline bool isSorted(int y) const {
        this->checkY(y);
        return fSorted[y];
    }

    // Sort right before blitting to make the memory hot
    void sort(int y) {
        this->checkY(y);
        if (!fSorted[y]) {
            SkTQSort(fRows[y], fRows[y] + fCounts[y] - 1);
            fSorted[y] = true;
        }
    }

    inline void addDelta(int x, int y, SkFixed delta) {
        this->push_back(y, {x, delta});
    }

private:
    SkCoverageDeltaAllocator*   fAlloc;
    SkCoverageDelta**           fRows;
    bool*                       fSorted;
    int*                        fCounts;
    int*                        fMaxCounts;
    int                         fTop;
    int                         fBottom;

    inline void checkY(int y) const {
        SkASSERT(y >= fTop && y < fBottom);
    }
};

class SkCoverageDeltaMask {
public:
    static constexpr int MAX_MASK_SIZE = 1024;

    static inline int ExpandWidth(int width) {
        int result = width + 2;
        return result + (4 - result % 4) % 4;
    }

    static bool Suitable(const SkIRect& bounds) {
        // 2 more width so we don't have to worry about the boundary
        return ExpandWidth(bounds.width()) * bounds.height() < MAX_MASK_SIZE;
    }

    SkCoverageDeltaMask(const SkIRect& bounds) : fBounds(bounds) {
        SkASSERT(Suitable(bounds));
        fExpandedWidth = ExpandWidth(fBounds.width());
        memset(fDeltaStorage, 0, sizeof(fDeltaStorage));

        fDeltas = fDeltaStorage;

        // Add one column so we may access fDeltas[index(-1, 0)]
        fDeltas += 1;

        // So we can directly access fDeltas[index(x, y)]
        fDeltas -= fBounds.fTop * fExpandedWidth + fBounds.fLeft;
    }

    inline int index(int x, int y) const {
        return y * fExpandedWidth + x;
    }

    inline SkFixed* row(int y) {
        this->checkY(y);
        return fDeltas + this->index(fBounds.fLeft, y);
    }

    inline SkFixed& delta(int x, int y) {
        this->checkX(x);
        this->checkY(y);
        return fDeltas[this->index(x, y)];
    }

    inline void addDelta(int x, int y, SkFixed delta) {
        this->delta(x, y) += delta;
    }

    inline int top() const {
        return fBounds.fTop;
    }

    inline int bottom() const {
        return fBounds.fBottom;
    }

    void convertCoverageToAlpha(bool isEvenOdd, bool isInverse, bool isConvex) {
        // todo future: we could really use some SIMD here?
        SkFixed* deltaRow = &this->delta(fBounds.fLeft, fBounds.fTop);
        SkAlpha* maskRow = fMask;
        for(int iy = 0; iy < fBounds.height(); ++iy) {
            SkFixed c[4] = {0, 0, 0, 0};
            for(int ix = 0; ix < fExpandedWidth; ix += 4) {
                c[0] = c[3] + deltaRow[ix];
                c[1] = c[0] + deltaRow[ix + 1];
                c[2] = c[1] + deltaRow[ix + 2];
                c[3] = c[2] + deltaRow[ix + 3];
                // My SIMD CoverageToAlpha seems to be only faster with SSSE3.
                // (On linux, even with -mavx2, my SIMD still seems to be slow...)
                // However, with SSSE2, it's faster to do 4 non-SIMD computations at one time.
                // Maybe the compiler is doing some SIMD by itself.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
                Sk4i c4 = Sk4i::Load(c);
                Sk4i a4i = isConvex ? ConvexCoverageToAlpha(c4, isInverse)
                                    : CoverageToAlpha(c4, isEvenOdd, isInverse);
                Sk4b a4 = SkNx_cast<SkAlpha>(a4i);
                a4.store(maskRow + ix);
#else
                for(int j = 0; j < 4; ++j) {
                    maskRow[ix + j] = isConvex ? ConvexCoverageToAlpha(c[j], isInverse)
                                               : CoverageToAlpha(c[j], isEvenOdd, isInverse);
                }
#endif
            }
            deltaRow += fExpandedWidth;
            maskRow += fBounds.width();
        }
    }

    inline SkAlpha* getMask() {
        return fMask;
    }

    inline const SkIRect& getBounds() const {
        return fBounds;
    }

private:
    SkIRect                     fBounds;
    SkFixed                     fDeltaStorage[MAX_MASK_SIZE];
    SkFixed*                    fDeltas;
    SkAlpha                     fMask[MAX_MASK_SIZE];
    int                         fExpandedWidth;

    void checkY(int y) const {
        SkASSERT(y >= fBounds.fTop - 1 && y <= fBounds.fBottom);
    }

    void checkX(int x) const {
        SkASSERT(x >= fBounds.fLeft - 1 && x <= fBounds.fRight);
    }
};

#endif
