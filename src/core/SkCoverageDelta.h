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

// Future todo: maybe we can make fX and fDelta 16-bit long to speed it up a little bit.
struct SkCoverageDelta {
    int     fX;     // the y coordinate will be implied in SkCoverageDeltaList
    SkFixed fDelta; // the amount that the alpha changed

    // Sort according to fX
    bool operator<(const SkCoverageDelta& other) const {
        return fX < other.fX;
    }
};

// All the arguments needed for SkBlitter::blitAntiRect
struct SkAntiRect {
    int     fX;
    int     fY;
    int     fWidth;
    int     fHeight;
    SkAlpha fLeftAlpha;
    SkAlpha fRightAlpha;
};

static SK_ALWAYS_INLINE SkAlpha CoverageToAlpha(SkFixed coverage, bool isEvenOdd, bool isInverse) {
    SkAlpha result;
    if (isEvenOdd) {
        SkFixed mod17 = coverage & 0x1ffff;
        SkFixed mod16 = coverage & 0xffff;
        result = SkTPin(SkAbs32((mod16 << 1) - mod17) >> 8, 0, 255);
    } else {
        result = SkTPin(SkAbs32(coverage) >> 8, 0, 255);
    }
    return isInverse ? 255 - result : result;
}

template<typename T>
static SK_ALWAYS_INLINE T CoverageToAlpha(T coverage, bool isEvenOdd, bool isInverse) {
    T t0(0), t255(255);
    T result;
    if (isEvenOdd) {
        T mod17 = coverage & 0x1ffff;
        T mod16 = coverage & 0xffff;
        result = ((mod16 << 1) - mod17).abs() >> 8;
    } else {
        result = coverage.abs() >> 8;;
    }
    result = T::Min(result, t255);
    result = T::Max(result, t0);
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

template<typename T>
static SK_ALWAYS_INLINE T ConvexCoverageToAlpha(T coverage, bool isInverse) {
    // allTrue is not implemented
    // SkASSERT((coverage >= -SK_Fixed1).allTrue() && (coverage <= SK_Fixed1).allTrue());
    T result = coverage.abs() >> 8;
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
    static constexpr int INIT_ROW_SIZE = 32;
    static constexpr int RESERVED_HEIGHT = 128;

    SkCoverageDeltaList(SkCoverageDeltaAllocator* alloc, int top, int bottom) {
        fAlloc = alloc;
        fTop = top;
        fBottom = bottom;

        // Init the anti-rect to be empty
        fAntiRect.fY = bottom;
        fAntiRect.fHeight = 0;

        if (bottom - top <= RESERVED_HEIGHT) {
            fSorted = fReservedSorted;
            fCounts = fReservedCounts;
            fMaxCounts = fReservedMaxCounts;
            fRows = fReservedRows - top;
            fRows[top] = fReservedStorage;
        } else {
            fSorted = fAlloc->makeArrayDefault<bool>(bottom - top);
            fCounts = fAlloc->makeArrayDefault<int>((bottom - top) * 2);
            fMaxCounts = fCounts + bottom - top;
            fRows = fAlloc->makeArrayDefault<SkCoverageDelta*>(bottom - top) - top;
            fRows[top] = fAlloc->makeArrayDefault<SkCoverageDelta>(INIT_ROW_SIZE * (bottom - top));
        }

        // Minus top so we can directly use fCounts[y] instead of fCounts[y - fTop].
        // Same for fMaxCounts, fRows, and fSorted.
        memset(fSorted, true, bottom - top);
        fSorted -= top;

        memset(fCounts, 0, sizeof(int) * (bottom - top));
        fCounts -= top;
        fMaxCounts -= top;
        for(int y = top; y < bottom; ++y) {
            fMaxCounts[y] = INIT_ROW_SIZE;
        }

        for(int y = top + 1; y < bottom; ++y) {
            fRows[y] = fRows[y - 1] + INIT_ROW_SIZE;
        }
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
        fSorted[y] = fSorted[y] && (fCounts[y] == 1 || delta.fX >= fRows[y][fCounts[y] - 2].fX);
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

    inline void setAntiRect(int x, int y, int width, int height,
            SkAlpha leftAlpha, SkAlpha rightAlpha) {
        fAntiRect = {x, y, width, height, leftAlpha, rightAlpha};
    }

    const SkAntiRect& getAntiRect() const {
        return fAntiRect;
    }

private:
    SkCoverageDeltaAllocator*   fAlloc;
    SkCoverageDelta**           fRows;
    bool*                       fSorted;
    int*                        fCounts;
    int*                        fMaxCounts;
    int                         fTop;
    int                         fBottom;
    SkAntiRect                  fAntiRect;

    SkCoverageDelta             fReservedStorage[RESERVED_HEIGHT * INIT_ROW_SIZE];
    SkCoverageDelta*            fReservedRows[RESERVED_HEIGHT];
    bool                        fReservedSorted[RESERVED_HEIGHT];
    int                         fReservedCounts[RESERVED_HEIGHT];
    int                         fReservedMaxCounts[RESERVED_HEIGHT];

    inline void checkY(int y) const {
        SkASSERT(y >= fTop && y < fBottom);
    }
};

class SkCoverageDeltaMask {
public:
    static constexpr int MAX_MASK_SIZE = 2048;
    static constexpr int SUITABLE_WIDTH = 32;

    // 1 for precision error, 1 for boundary delta (e.g., -SK_Fixed1 at fBounds.fRight + 1)
    static constexpr int PADDING = 2;

    static constexpr int SIMD_WIDTH = 8;

    // Expand PADDING on both sides, and make it a multiple of SIMD_WIDTH
    static inline int ExpandWidth(int width) {
        int result = width + PADDING * 2;
        return result + (SIMD_WIDTH - result % SIMD_WIDTH) % SIMD_WIDTH;
    }

    static inline bool CanHandle(const SkIRect& bounds) {
        // 2 more width so we don't have to worry about the boundary
        return ExpandWidth(bounds.width()) * bounds.height() < MAX_MASK_SIZE;
    }

    static bool Suitable(const SkIRect& bounds) {
        return bounds.width() <= SUITABLE_WIDTH && CanHandle(bounds);
    }

    SkCoverageDeltaMask(const SkIRect& bounds) : fBounds(bounds) {
        SkASSERT(CanHandle(bounds));
        fExpandedWidth = ExpandWidth(fBounds.width());
        memset(fDeltaStorage, 0, sizeof(fDeltaStorage));

        fDeltas = fDeltaStorage;

        // Add PADDING columns so we may access fDeltas[index(-PADDING, 0)]
        fDeltas += PADDING;

        // So we can directly access fDeltas[index(x, y)]
        fDeltas -= fBounds.fTop * fExpandedWidth + fBounds.fLeft;

        // Init the anti-rect to be empty
        fAntiRect.fY = fBounds.fBottom;
        fAntiRect.fHeight = 0;
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

    inline void setAntiRect(int x, int y, int width, int height,
            SkAlpha leftAlpha, SkAlpha rightAlpha) {
        fAntiRect = {x, y, width, height, leftAlpha, rightAlpha};
    }

    void convertCoverageToAlpha(bool isEvenOdd, bool isInverse, bool isConvex) {
        SkFixed* deltaRow = &this->delta(fBounds.fLeft, fBounds.fTop);
        SkAlpha* maskRow = fMask;
        for(int iy = 0; iy < fBounds.height(); ++iy) {
            if (fAntiRect.fHeight && iy == fAntiRect.fY - fBounds.fTop) {
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
                    deltaRow += fExpandedWidth;
                    maskRow += fBounds.width();
                }
                iy += fAntiRect.fHeight - 1;
                continue;
            }
            SkFixed c[SIMD_WIDTH] = {0};
            for(int ix = 0; ix < fExpandedWidth; ix += SIMD_WIDTH) {
                // Future todo: is it faster to process SIMD_WIDTH rows at a time so we can use SIMD
                // for coverage accumulation?
                c[0] = c[SIMD_WIDTH - 1] + deltaRow[ix];
                for(int j = 1; j < SIMD_WIDTH; ++j) {
                    c[j] = c[j - 1] + deltaRow[ix + j];
                }
                // My SIMD CoverageToAlpha seems to be only faster with SSSE3.
                // (On linux, even with -mavx2, my SIMD still seems to be slow...)
                // However, with SSSE2, it's faster to do SIMD_WIDTH non-SIMD computations at one
                // time. Maybe the compiler is doing some SIMD by itself.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
                using SkNi = SkNx<SIMD_WIDTH, int>;
                using SkNb = SkNx<SIMD_WIDTH, SkAlpha>;
                SkNi cn = SkNi::Load(c);
                SkNi ani = isConvex ? ConvexCoverageToAlpha(cn, isInverse)
                                    : CoverageToAlpha(cn, isEvenOdd, isInverse);
                SkNb an = SkNx_cast<SkAlpha>(ani);
                an.store(maskRow + ix);
#else
                for(int j = 0; j < SIMD_WIDTH; ++j) {
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
    SkAntiRect                  fAntiRect;

    void checkY(int y) const {
        SkASSERT(y >= fBounds.fTop && y < fBounds.fBottom);
    }

    void checkX(int x) const {
        SkASSERT(x >= fBounds.fLeft - PADDING && x < fBounds.fRight + PADDING);
    }
};

#endif
