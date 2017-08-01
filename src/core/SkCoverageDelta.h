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
#include "SkMask.h"
#include "SkTSort.h"
#include "SkUtils.h"

// Future todo: maybe we can make fX and fDelta 16-bit long to speed it up a little bit.
struct SkCoverageDelta {
    int16_t fX;
    int16_t fY;
    SkFixed fDelta; // the amount that the alpha changed
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

using SkCoverageDeltaAllocator = SkSTArenaAlloc<256>;

// A list of SkCoverageDelta with y from top() to bottom().
// For each row y, there are count(y) number of deltas.
// You can ask whether they are sorted or not by sorted(y), and you can sort them by sort(y).
// Once sorted, getDelta(y, i) should return the i-th leftmost delta on row y.
class SkCoverageDeltaList {
public:
    static constexpr int PADDING = 1; // for precision error

    // We can store INIT_ROW_SIZE deltas per row (i.e., per y-scanline) initially,
    // and we reserve RESERVED_HEIGHT rows on stack memory.
#ifdef GOOGLE3
    static constexpr int INIT_ROW_SIZE = 8; // google3 has 16k stack limit
    static constexpr int RESERVED_HEIGHT = 120;
#else
    static constexpr int RESERVED_WIDTH     = 128;
    static constexpr int RESERVED_HEIGHT    = 128;
    static constexpr int RESERVED_STORAGE   = 8 * 1024;
#endif

    SkCoverageDeltaList(SkCoverageDeltaAllocator* alloc, const SkIRect& bounds, bool forceRLE);

    inline int  top() const { return fBounds.fTop; }
    inline int  bottom() const { return fBounds.fBottom; }
    inline bool forceRLE() const { return fForceRLE; }
    inline int  countByX(int x) const { this->checkX(x); return fCountByX[x]; }
    inline int  countByY(int y) const { this->checkY(y); return fCountByY[y]; }
    inline void addDelta(int x, int y, SkFixed delta) {
        SkASSERT((int16_t)x == x && (int16_t)y == y);
        this->push_back({(int16_t)x, (int16_t)y, delta});
    }

    inline const SkCoverageDelta& getDelta(int y, int i) const {
        this->checkY(y);
        SkASSERT(fRows && i < fCountByY[y]);
        return fRows[y][i];
    }

    inline const SkAntiRect& getAntiRect() const { return fAntiRect; }
    inline void setAntiRect(int x, int y, int width, int height,
            SkAlpha leftAlpha, SkAlpha rightAlpha) {
        fAntiRect = {x, y, width, height, leftAlpha, rightAlpha};
    }

    inline void push_back(const SkCoverageDelta& delta) {
        if (fCount == fMaxCount) {
            fMaxCount *= 2;
            SkCoverageDelta* deltas = fAlloc->makeArrayDefault<SkCoverageDelta>(fMaxCount);
            memcpy(deltas, fDeltas, sizeof(SkCoverageDelta) * fCount);
            fDeltas = deltas;
        }
        fDeltas[fCount++] = delta;
        fCountByX[delta.fX]++;
        fCountByY[delta.fY]++;
    }

    void sort();

private:
    SkCoverageDeltaAllocator*   fAlloc;
    SkCoverageDelta*            fDeltas;
    SkCoverageDelta**           fRows;
    int*                        fCountByX;
    int*                        fCountByY;
    int                         fCount;
    int                         fMaxCount;
    SkIRect                     fBounds;
    SkAntiRect                  fAntiRect;
    bool                        fForceRLE;

    SkCoverageDelta             fReservedStorage[RESERVED_STORAGE];
    SkCoverageDelta*            fReservedRows[RESERVED_HEIGHT];
    int                         fReservedCounts[RESERVED_HEIGHT + RESERVED_WIDTH];

    inline void checkX(int x) const { SkASSERT(x >= fBounds.fLeft && x < fBounds.fRight); }
    inline void checkY(int y) const { SkASSERT(y >= fBounds.fTop && y < fBounds.fBottom); }
};

class SkCoverageDeltaMask {
public:
    // 1 for precision error, 1 for boundary delta (e.g., -SK_Fixed1 at fBounds.fRight + 1)
    static constexpr int PADDING        = 2;

    static constexpr int SIMD_WIDTH     = 8;
    static constexpr int SUITABLE_WIDTH = 32;
#ifdef GOOGLE3
    static constexpr int MAX_MASK_SIZE  = 1024; // G3 has 16k stack limit based on -fstack-usage
#else
    static constexpr int MAX_MASK_SIZE  = 2048;
#endif

    // Expand PADDING on both sides, and make it a multiple of SIMD_WIDTH
    static int  ExpandWidth(int width);
    static bool CanHandle(const SkIRect& bounds);   // whether bounds fits into MAX_MASK_SIZE
    static bool Suitable(const SkIRect& bounds);    // CanHandle(bounds) && width <= SUITABLE_WIDTH

    SkCoverageDeltaMask(const SkIRect& bounds);

    inline int              top()       const { return fBounds.fTop; }
    inline int              bottom()    const { return fBounds.fBottom; }
    inline SkAlpha*         getMask()         { return fMask; }
    inline const SkIRect&   getBounds() const { return fBounds; }

    inline void             addDelta (int x, int y, SkFixed delta) { this->delta(x, y) += delta; }
    inline SkFixed&         delta    (int x, int y) {
        this->checkX(x);
        this->checkY(y);
        return fDeltas[this->index(x, y)];
    }

    inline void setAntiRect(int x, int y, int width, int height,
                            SkAlpha leftAlpha, SkAlpha rightAlpha) {
        fAntiRect = {x, y, width, height, leftAlpha, rightAlpha};
    }

    inline SkMask prepareSkMask() {
        SkMask mask;
        mask.fImage     = fMask;
        mask.fBounds    = fBounds;
        mask.fRowBytes  = fBounds.width();
        mask.fFormat    = SkMask::kA8_Format;
        return mask;
    }

    void convertCoverageToAlpha(bool isEvenOdd, bool isInverse, bool isConvex);

private:
    SkIRect     fBounds;
    SkFixed     fDeltaStorage[MAX_MASK_SIZE];
    SkFixed*    fDeltas;
    SkAlpha     fMask[MAX_MASK_SIZE];
    int         fExpandedWidth;
    SkAntiRect  fAntiRect;

    inline int  index(int x, int y) const { return y * fExpandedWidth + x; }
    inline void checkY(int y) const { SkASSERT(y >= fBounds.fTop && y < fBounds.fBottom); }
    inline void checkX(int x) const {
        SkASSERT(x >= fBounds.fLeft - PADDING && x < fBounds.fRight + PADDING);
    }
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
// between [-SK_Fixed1, SK_Fixed1] so we can skip isEvenOdd and SkTPin.
static SK_ALWAYS_INLINE SkAlpha ConvexCoverageToAlpha(SkFixed coverage, bool isInverse) {
    SkASSERT(coverage >= -SK_Fixed1 && coverage <= SK_Fixed1);
    int result = SkAbs32(coverage) >> 8;
    result -= (result >> 8); // 256 to 255
    return isInverse ? 255 - result : result;
}

template<typename T>
static SK_ALWAYS_INLINE T ConvexCoverageToAlpha(T coverage, bool isInverse) {
    // allTrue is not implemented
    // SkASSERT((coverage >= 0).allTrue() && (coverage <= SK_Fixed1).allTrue());
    T result = coverage.abs() >> 8;
    result -= (result >> 8); // 256 to 255
    return isInverse ? 255 - result : result;
}

#endif
