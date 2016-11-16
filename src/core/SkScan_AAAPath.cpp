/*
 * Copyright 2016 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAntiRun.h"
#include "SkBlitter.h"
#include "SkEdge.h"
#include "SkAnalyticEdge.h"
#include "SkEdgeBuilder.h"
#include "SkGeometry.h"
#include "SkPath.h"
#include "SkQuadClipper.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "SkScan.h"
#include "SkScanPriv.h"
#include "SkTemplates.h"
#include "SkTSort.h"
#include "SkUtils.h"

///////////////////////////////////////////////////////////////////////////////

/*

The following is a high-level overview of our analytic anti-aliasing
algorithm. We consider a path as a collection of line segments, as
quadratic/cubic curves are converted to small line segments. Without loss of
generality, let's assume that the draw region is [0, W] x [0, H].

Our algorithm is based on horizontal scan lines (y = c_i) as the previous
sampling-based algorithm did. However, our algorithm uses non-equal-spaced
scan lines, while the previous method always uses equal-spaced scan lines,
such as (y = 1/2 + 0, 1/2 + 1, 1/2 + 2, ...) in the previous non-AA algorithm,
and (y = 1/8 + 1/4, 1/8 + 2/4, 1/8 + 3/4, ...) in the previous
16-supersampling AA algorithm.

Our algorithm contains scan lines y = c_i for c_i that is either:

1. an integer between [0, H]

2. the y value of a line segment endpoint

3. the y value of an intersection of two line segments

For two consecutive scan lines y = c_i, y = c_{i+1}, we analytically computes
the coverage of this horizontal strip of our path on each pixel. This can be
done very efficiently because the strip of our path now only consists of
trapezoids whose top and bottom edges are y = c_i, y = c_{i+1} (this includes
rectangles and triangles as special cases).

We now describe how the coverage of single pixel is computed against such a
trapezoid. That coverage is essentially the intersection area of a rectangle
(e.g., [0, 1] x [c_i, c_{i+1}]) and our trapezoid. However, that intersection
could be complicated, as shown in the example region A below:

+-----------\----+
|            \  C|
|             \  |
\              \ |
|\      A       \|
| \              \
|  \             |
| B \            |
+----\-----------+

However, we don't have to compute the area of A directly. Instead, we can
compute the excluded area, which are B and C, quite easily, because they're
just triangles. In fact, we can prove that an excluded region (take B as an
example) is either itself a simple trapezoid (including rectangles, triangles,
and empty regions), or its opposite (the opposite of B is A + C) is a simple
trapezoid. In any case, we can compute its area efficiently.

In summary, our algorithm has a higher quality because it generates ground-
truth coverages analytically. It is also faster because it has much fewer
unnessasary horizontal scan lines. For example, given a triangle path, the
number of scan lines in our algorithm is only about 3 + H while the
16-supersampling algorithm has about 4H scan lines.

*/

///////////////////////////////////////////////////////////////////////////////

static inline void addAlpha(SkAlpha& alpha, SkAlpha delta) {
    SkASSERT(alpha + (int)delta <= 256);
    alpha = SkAlphaRuns::CatchOverflow(alpha + (int)delta);
}

class AdditiveBlitter : public SkBlitter {
public:
    virtual ~AdditiveBlitter() {}

    virtual SkBlitter* getRealBlitter(bool forceRealBlitter = false) = 0;

    virtual void blitAntiH(int x, int y, const SkAlpha antialias[], int len) = 0;
    virtual void blitAntiH(int x, int y, const SkAlpha alpha) = 0;
    virtual void blitAntiH(int x, int y, int width, const SkAlpha alpha) = 0;

    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override {
        SkDEBUGFAIL("Please call real blitter's blitAntiH instead.");
    }

    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkDEBUGFAIL("Please call real blitter's blitV instead.");
    }

    void blitH(int x, int y, int width) override {
        SkDEBUGFAIL("Please call real blitter's blitH instead.");
    }

    void blitRect(int x, int y, int width, int height) override {
        SkDEBUGFAIL("Please call real blitter's blitRect instead.");
    }

    void blitAntiRect(int x, int y, int width, int height,
                      SkAlpha leftAlpha, SkAlpha rightAlpha) override {
        SkDEBUGFAIL("Please call real blitter's blitAntiRect instead.");
    }

    virtual int getWidth() = 0;

    // Flush the additive alpha cache if floor(y) and floor(nextY) is different
    // (i.e., we'll start working on a new pixel row).
    virtual void flush_if_y_changed(SkFixed y, SkFixed nextY) = 0;
};

// We need this mask blitter because it significantly accelerates small path filling.
class MaskAdditiveBlitter : public AdditiveBlitter {
public:
    MaskAdditiveBlitter(SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip,
            bool isInverse);
    ~MaskAdditiveBlitter() {
        fRealBlitter->blitMask(fMask, fClipRect);
    }

    // Most of the time, we still consider this mask blitter as the real blitter
    // so we can accelerate blitRect and others. But sometimes we want to return
    // the absolute real blitter (e.g., when we fall back to the old code path).
    SkBlitter* getRealBlitter(bool forceRealBlitter) override {
        return forceRealBlitter ? fRealBlitter : this;
    }

    // Virtual function is slow. So don't use this. Directly add alpha to the mask instead.
    void blitAntiH(int x, int y, const SkAlpha antialias[], int len) override;

    // Allowing following methods are used to blit rectangles during aaa_walk_convex_edges
    // Since there aren't many rectangles, we can still break the slow speed of virtual functions.
    void blitAntiH(int x, int y, const SkAlpha alpha) override;
    void blitAntiH(int x, int y, int width, const SkAlpha alpha) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitAntiRect(int x, int y, int width, int height,
                      SkAlpha leftAlpha, SkAlpha rightAlpha) override;

    // The flush is only needed for RLE (RunBasedAdditiveBlitter)
    void flush_if_y_changed(SkFixed y, SkFixed nextY) override {}

    int getWidth() override { return fClipRect.width(); }

    static bool canHandleRect(const SkIRect& bounds) {
        int width = bounds.width();
        if (width > MaskAdditiveBlitter::kMAX_WIDTH) {
            return false;
        }
        int64_t rb = SkAlign4(width);
        // use 64bits to detect overflow
        int64_t storage = rb * bounds.height();

        return (width <= MaskAdditiveBlitter::kMAX_WIDTH) &&
               (storage <= MaskAdditiveBlitter::kMAX_STORAGE);
    }

    // Return a pointer where pointer[x] corresonds to the alpha of (x, y)
    inline uint8_t* getRow(int y) {
        if (y != fY) {
            fY = y;
            fRow = fMask.fImage + (y - fMask.fBounds.fTop) * fMask.fRowBytes - fMask.fBounds.fLeft;
        }
        return fRow;
    }

private:
    // so we don't try to do very wide things, where the RLE blitter would be faster
    static const int kMAX_WIDTH = 32;
    static const int kMAX_STORAGE = 1024;

    SkBlitter*  fRealBlitter;
    SkMask      fMask;
    SkIRect     fClipRect;
    // we add 2 because we can write 1 extra byte at either end due to precision error
    uint32_t    fStorage[(kMAX_STORAGE >> 2) + 2];

    uint8_t*    fRow;
    int         fY;
};

MaskAdditiveBlitter::MaskAdditiveBlitter(
        SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip, bool isInverse) {
    SkASSERT(canHandleRect(ir));
    SkASSERT(!isInverse);

    fRealBlitter = realBlitter;

    fMask.fImage    = (uint8_t*)fStorage + 1; // There's 1 extra byte at either end of fStorage
    fMask.fBounds   = ir;
    fMask.fRowBytes = ir.width();
    fMask.fFormat   = SkMask::kA8_Format;

    fY = ir.fTop - 1;
    fRow = nullptr;

    fClipRect = ir;
    if (!fClipRect.intersect(clip.getBounds())) {
        SkASSERT(0);
        fClipRect.setEmpty();
    }

    memset(fStorage, 0, fMask.fBounds.height() * fMask.fRowBytes + 2);
}

void MaskAdditiveBlitter::blitAntiH(int x, int y, const SkAlpha antialias[], int len) {
    SkFAIL("Don't use this; directly add alphas to the mask.");
}

void MaskAdditiveBlitter::blitAntiH(int x, int y, const SkAlpha alpha) {
    SkASSERT(x >= fMask.fBounds.fLeft -1);
    addAlpha(this->getRow(y)[x], alpha);
}

void MaskAdditiveBlitter::blitAntiH(int x, int y, int width, const SkAlpha alpha) {
    SkASSERT(x >= fMask.fBounds.fLeft -1);
    uint8_t* row = this->getRow(y);
    for (int i=0; i<width; i++) {
        addAlpha(row[x + i], alpha);
    }
}

void MaskAdditiveBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 0) {
        return;
    }
    SkASSERT(x >= fMask.fBounds.fLeft -1);
    // This must be called as if this is a real blitter.
    // So we directly set alpha rather than adding it.
    uint8_t* row = this->getRow(y);
    for (int i=0; i<height; i++) {
        row[x] = alpha;
        row += fMask.fRowBytes;
    }
}

void MaskAdditiveBlitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= fMask.fBounds.fLeft -1);
    // This must be called as if this is a real blitter.
    // So we directly set alpha rather than adding it.
    uint8_t* row = this->getRow(y);
    for (int i=0; i<height; i++) {
        memset(row + x, 0xFF, width);
        row += fMask.fRowBytes;
    }
}

void MaskAdditiveBlitter::blitAntiRect(int x, int y, int width, int height,
        SkAlpha leftAlpha, SkAlpha rightAlpha) {
    blitV(x, y, height, leftAlpha);
    blitV(x + 1 + width, y, height, rightAlpha);
    blitRect(x + 1, y, width, height);
}

class RunBasedAdditiveBlitter : public AdditiveBlitter {
public:
    RunBasedAdditiveBlitter(SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip,
            bool isInverse);
    ~RunBasedAdditiveBlitter();

    SkBlitter* getRealBlitter(bool forceRealBlitter) override;

    void blitAntiH(int x, int y, const SkAlpha antialias[], int len) override;
    void blitAntiH(int x, int y, const SkAlpha alpha) override;
    void blitAntiH(int x, int y, int width, const SkAlpha alpha) override;

    int getWidth() override;

    void flush_if_y_changed(SkFixed y, SkFixed nextY) override {
        if (SkFixedFloorToInt(y) != SkFixedFloorToInt(nextY)) {
            this->flush();
        }
    }

private:
    SkBlitter* fRealBlitter;

    /// Current y coordinate
    int         fCurrY;
    /// Widest row of region to be blitted
    int         fWidth;
    /// Leftmost x coordinate in any row
    int         fLeft;
    /// Initial y coordinate (top of bounds).
    int         fTop;

    // The next three variables are used to track a circular buffer that
    // contains the values used in SkAlphaRuns. These variables should only
    // ever be updated in advanceRuns(), and fRuns should always point to
    // a valid SkAlphaRuns...
    int         fRunsToBuffer;
    void*       fRunsBuffer;
    int         fCurrentRun;
    SkAlphaRuns fRuns;

    int         fOffsetX;

    inline bool check(int x, int width) {
        #ifdef SK_DEBUG
        if (x < 0 || x + width > fWidth) {
            SkDebugf("Ignore x = %d, width = %d\n", x, width);
        }
        #endif
        return (x >= 0 && x + width <= fWidth);
    }

    // extra one to store the zero at the end
    inline int getRunsSz() const { return (fWidth + 1 + (fWidth + 2)/2) * sizeof(int16_t); }

    // This function updates the fRuns variable to point to the next buffer space
    // with adequate storage for a SkAlphaRuns. It mostly just advances fCurrentRun
    // and resets fRuns to point to an empty scanline.
    inline void advanceRuns() {
        const size_t kRunsSz = this->getRunsSz();
        fCurrentRun = (fCurrentRun + 1) % fRunsToBuffer;
        fRuns.fRuns = reinterpret_cast<int16_t*>(
            reinterpret_cast<uint8_t*>(fRunsBuffer) + fCurrentRun * kRunsSz);
        fRuns.fAlpha = reinterpret_cast<SkAlpha*>(fRuns.fRuns + fWidth + 1);
        fRuns.reset(fWidth);
    }

    // Blitting 0xFF and 0 is much faster so we snap alphas close to them
    inline SkAlpha snapAlpha(SkAlpha alpha) {
        return alpha > 247 ? 0xFF : alpha < 8 ? 0 : alpha;
    }

    inline void flush() {
        if (fCurrY >= fTop) {
            SkASSERT(fCurrentRun < fRunsToBuffer);
            for (int x = 0; fRuns.fRuns[x]; x += fRuns.fRuns[x]) {
                // It seems that blitting 255 or 0 is much faster than blitting 254 or 1
                fRuns.fAlpha[x] = snapAlpha(fRuns.fAlpha[x]);
            }
            if (!fRuns.empty()) {
                // SkDEBUGCODE(fRuns.dump();)
                fRealBlitter->blitAntiH(fLeft, fCurrY, fRuns.fAlpha, fRuns.fRuns);
                this->advanceRuns();
                fOffsetX = 0;
            }
            fCurrY = fTop - 1;
        }
    }

    inline void checkY(int y) {
        if (y != fCurrY) {
            this->flush();
            fCurrY = y;
        }
    }
};

RunBasedAdditiveBlitter::RunBasedAdditiveBlitter(
        SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip, bool isInverse) {
    fRealBlitter = realBlitter;

    SkIRect sectBounds;
    if (isInverse) {
        // We use the clip bounds instead of the ir, since we may be asked to
        //draw outside of the rect when we're a inverse filltype
        sectBounds = clip.getBounds();
    } else {
        if (!sectBounds.intersect(ir, clip.getBounds())) {
            sectBounds.setEmpty();
        }
    }

    const int left = sectBounds.left();
    const int right = sectBounds.right();

    fLeft = left;
    fWidth = right - left;
    fTop = sectBounds.top();
    fCurrY = fTop - 1;

    fRunsToBuffer = realBlitter->requestRowsPreserved();
    fRunsBuffer = realBlitter->allocBlitMemory(fRunsToBuffer * this->getRunsSz());
    fCurrentRun = -1;

    this->advanceRuns();

    fOffsetX = 0;
}

RunBasedAdditiveBlitter::~RunBasedAdditiveBlitter() {
    this->flush();
}

SkBlitter* RunBasedAdditiveBlitter::getRealBlitter(bool forceRealBlitter) {
    return fRealBlitter;
}

void RunBasedAdditiveBlitter::blitAntiH(int x, int y, const SkAlpha antialias[], int len) {
    checkY(y);
    x -= fLeft;

    if (x < 0) {
        len += x;
        antialias -= x;
        x = 0;
    }
    len = SkTMin(len, fWidth - x);
    SkASSERT(check(x, len));

    if (x < fOffsetX) {
        fOffsetX = 0;
    }

    fOffsetX = fRuns.add(x, 0, len, 0, 0, fOffsetX); // Break the run
    for (int i = 0; i < len; i += fRuns.fRuns[x + i]) {
        for (int j = 1; j < fRuns.fRuns[x + i]; j++) {
            fRuns.fRuns[x + i + j] = 1;
            fRuns.fAlpha[x + i + j] = fRuns.fAlpha[x + i];
        }
        fRuns.fRuns[x + i] = 1;
    }
    for (int i=0; i<len; i++) {
        addAlpha(fRuns.fAlpha[x + i], antialias[i]);
    }
}
void RunBasedAdditiveBlitter::blitAntiH(int x, int y, const SkAlpha alpha) {
    checkY(y);
    x -= fLeft;

    if (x < fOffsetX) {
        fOffsetX = 0;
    }

    if (this->check(x, 1)) {
        fOffsetX = fRuns.add(x, 0, 1, 0, alpha, fOffsetX);
    }
}

void RunBasedAdditiveBlitter::blitAntiH(int x, int y, int width, const SkAlpha alpha) {
    checkY(y);
    x -= fLeft;

    if (x < fOffsetX) {
        fOffsetX = 0;
    }

    if (this->check(x, width)) {
        fOffsetX = fRuns.add(x, 0, width, 0, alpha, fOffsetX);
    }
}

int RunBasedAdditiveBlitter::getWidth() { return fWidth; }

///////////////////////////////////////////////////////////////////////////////

// Return the alpha of a trapezoid whose height is 1
static inline SkAlpha trapezoidToAlpha(SkFixed l1, SkFixed l2) {
    SkASSERT(l1 >= 0 && l2 >= 0);
    return ((l1 + l2) >> 9);
}

// The alpha of right-triangle (a, a*b), in 16 bits
static inline SkFixed partialTriangleToAlpha16(SkFixed a, SkFixed b) {
    SkASSERT(a <= SK_Fixed1);
    // SkFixedMul_lowprec(SkFixedMul_lowprec(a, a), b) >> 1
    // return ((((a >> 8) * (a >> 8)) >> 8) * (b >> 8)) >> 1;
    return (a >> 11) * (a >> 11) * (b >> 11);
}

// The alpha of right-triangle (a, a*b)
static inline SkAlpha partialTriangleToAlpha(SkFixed a, SkFixed b) {
    return partialTriangleToAlpha16(a, b) >> 8;
}

static inline SkAlpha getPartialAlpha(SkAlpha alpha, SkFixed partialHeight) {
    return SkToU8(SkFixedRoundToInt(alpha * partialHeight));
}

static inline SkAlpha getPartialAlpha(SkAlpha alpha, SkAlpha fullAlpha) {
    return ((uint16_t)alpha * fullAlpha) >> 8;
}

// For SkFixed that's close to SK_Fixed1, we can't convert it to alpha by just shifting right.
// For example, when f = SK_Fixed1, right shifting 8 will get 256, but we need 255.
// This is rarely the problem so we'll only use this for blitting rectangles.
static inline SkAlpha f2a(SkFixed f) {
    SkASSERT(f <= SK_Fixed1);
    return getPartialAlpha(0xFF, f);
}

// Suppose that line (l1, y)-(r1, y+1) intersects with (l2, y)-(r2, y+1),
// approximate (very coarsely) the x coordinate of the intersection.
static inline SkFixed approximateIntersection(SkFixed l1, SkFixed r1, SkFixed l2, SkFixed r2) {
    if (l1 > r1) { SkTSwap(l1, r1); }
    if (l2 > r2) { SkTSwap(l2, r2); }
    return (SkTMax(l1, l2) + SkTMin(r1, r2)) >> 1;
}

// Here we always send in l < SK_Fixed1, and the first alpha we want to compute is alphas[0]
static inline void computeAlphaAboveLine(SkAlpha* alphas, SkFixed l, SkFixed r,
                                         SkFixed dY, SkAlpha fullAlpha) {
    SkASSERT(l <= r);
    SkASSERT(l >> 16 == 0);
    int R = SkFixedCeilToInt(r);
    if (R == 0) {
        return;
    } else if (R == 1) {
        alphas[0] = getPartialAlpha(((R << 17) - l - r) >> 9, fullAlpha);
    } else {
        SkFixed first = SK_Fixed1 - l; // horizontal edge length of the left-most triangle
        SkFixed last = r - ((R - 1) << 16); // horizontal edge length of the right-most triangle
        SkFixed firstH = SkFixedMul_lowprec(first, dY); // vertical edge of the left-most triangle
        alphas[0] = SkFixedMul_lowprec(first, firstH) >> 9; // triangle alpha
        SkFixed alpha16 = firstH + (dY >> 1); // rectangle plus triangle
        for (int i = 1; i < R - 1; i++) {
            alphas[i] = alpha16 >> 8;
            alpha16 += dY;
        }
        alphas[R - 1] = fullAlpha - partialTriangleToAlpha(last, dY);
    }
}

// Here we always send in l < SK_Fixed1, and the first alpha we want to compute is alphas[0]
static inline void computeAlphaBelowLine(
        SkAlpha* alphas, SkFixed l, SkFixed r, SkFixed dY, SkAlpha fullAlpha) {
    SkASSERT(l <= r);
    SkASSERT(l >> 16 == 0);
    int R = SkFixedCeilToInt(r);
    if (R == 0) {
        return;
    } else if (R == 1) {
        alphas[0] = getPartialAlpha(trapezoidToAlpha(l, r), fullAlpha);
    } else {
        SkFixed first = SK_Fixed1 - l; // horizontal edge length of the left-most triangle
        SkFixed last = r - ((R - 1) << 16); // horizontal edge length of the right-most triangle
        SkFixed lastH = SkFixedMul_lowprec(last, dY); // vertical edge of the right-most triangle
        alphas[R-1] = SkFixedMul_lowprec(last, lastH) >> 9; // triangle alpha
        SkFixed alpha16 = lastH + (dY >> 1); // rectangle plus triangle
        for (int i = R - 2; i > 0; i--) {
            alphas[i] = alpha16 >> 8;
            alpha16 += dY;
        }
        alphas[0] = fullAlpha - partialTriangleToAlpha(first, dY);
    }
}

// Note that if fullAlpha != 0xFF, we'll multiply alpha by fullAlpha
static inline void blit_single_alpha(AdditiveBlitter* blitter, int y, int x,
                              SkAlpha alpha, SkAlpha fullAlpha, SkAlpha* maskRow,
                              bool isUsingMask) {
    if (isUsingMask) {
        if (fullAlpha == 0xFF) {
            maskRow[x] = alpha;
        } else {
            addAlpha(maskRow[x], getPartialAlpha(alpha, fullAlpha));
        }
    } else {
        if (fullAlpha == 0xFF) {
            blitter->getRealBlitter()->blitV(x, y, 1, alpha);
        } else {
            blitter->blitAntiH(x, y, getPartialAlpha(alpha, fullAlpha));
        }
    }
}

static inline void blit_two_alphas(AdditiveBlitter* blitter, int y, int x,
                            SkAlpha a1, SkAlpha a2, SkAlpha fullAlpha, SkAlpha* maskRow,
                            bool isUsingMask) {
    if (isUsingMask) {
        addAlpha(maskRow[x], a1);
        addAlpha(maskRow[x + 1], a2);
    } else {
        if (fullAlpha == 0xFF) {
            blitter->getRealBlitter()->blitAntiH2(x, y, a1, a2);
        } else {
            blitter->blitAntiH(x, y, a1);
            blitter->blitAntiH(x + 1, y, a2);
        }
    }
}

// It's important that this is inline. Otherwise it'll be much slower.
static SK_ALWAYS_INLINE void blit_full_alpha(AdditiveBlitter* blitter, int y, int x, int len,
                            SkAlpha fullAlpha, SkAlpha* maskRow, bool isUsingMask) {
    if (isUsingMask) {
        for (int i=0; i<len; i++) {
            addAlpha(maskRow[x + i], fullAlpha);
        }
    } else {
        if (fullAlpha == 0xFF) {
            blitter->getRealBlitter()->blitH(x, y, len);
        } else {
            blitter->blitAntiH(x, y, len, fullAlpha);
        }
    }
}

static void blit_aaa_trapezoid_row(AdditiveBlitter* blitter, int y,
                                   SkFixed ul, SkFixed ur, SkFixed ll, SkFixed lr,
                                   SkFixed lDY, SkFixed rDY, SkAlpha fullAlpha, SkAlpha* maskRow,
                                   bool isUsingMask) {
    int L = SkFixedFloorToInt(ul), R = SkFixedCeilToInt(lr);
    int len = R - L;

    if (len == 1) {
        SkAlpha alpha = trapezoidToAlpha(ur - ul, lr - ll);
        blit_single_alpha(blitter, y, L, alpha, fullAlpha, maskRow, isUsingMask);
        return;
    }

    // SkDebugf("y = %d, len = %d, ul = %f, ur = %f, ll = %f, lr = %f\n", y, len,
    //         SkFixedToFloat(ul), SkFixedToFloat(ur), SkFixedToFloat(ll), SkFixedToFloat(lr));

    const int kQuickLen = 31;
    // This is faster than SkAutoSMalloc<1024>
    char quickMemory[(sizeof(SkAlpha) * 2 + sizeof(int16_t)) * (kQuickLen + 1)];
    SkAlpha* alphas;

    if (len <= kQuickLen) {
        alphas = (SkAlpha*)quickMemory;
    } else {
        alphas = new SkAlpha[(len + 1) * (sizeof(SkAlpha) * 2 + sizeof(int16_t))];
    }

    SkAlpha* tempAlphas = alphas + len + 1;
    int16_t* runs = (int16_t*)(alphas + (len + 1) * 2);

    for (int i = 0; i < len; i++) {
        runs[i] = 1;
        alphas[i] = fullAlpha;
    }
    runs[len] = 0;

    int uL = SkFixedFloorToInt(ul);
    int lL = SkFixedCeilToInt(ll);
    if (uL + 2 == lL) { // We only need to compute two triangles, accelerate this special case
        SkFixed first = (uL << 16) + SK_Fixed1 - ul;
        SkFixed second = ll - ul - first;
        SkAlpha a1 = fullAlpha - partialTriangleToAlpha(first, lDY);
        SkAlpha a2 = partialTriangleToAlpha(second, lDY);
        alphas[0] = alphas[0] > a1 ? alphas[0] - a1 : 0;
        alphas[1] = alphas[1] > a2 ? alphas[1] - a2 : 0;
    } else {
        computeAlphaBelowLine(tempAlphas + uL - L, ul - (uL << 16), ll - (uL << 16),
                lDY, fullAlpha);
        for (int i = uL; i < lL; i++) {
            if (alphas[i - L] > tempAlphas[i - L]) {
                alphas[i - L] -= tempAlphas[i - L];
            } else {
                alphas[i - L] = 0;
            }
        }
    }

    int uR = SkFixedFloorToInt(ur);
    int lR = SkFixedCeilToInt(lr);
    if (uR + 2 == lR) { // We only need to compute two triangles, accelerate this special case
        SkFixed first = (uR << 16) + SK_Fixed1 - ur;
        SkFixed second = lr - ur - first;
        SkAlpha a1 = partialTriangleToAlpha(first, rDY);
        SkAlpha a2 = fullAlpha - partialTriangleToAlpha(second, rDY);
        alphas[len-2] = alphas[len-2] > a1 ? alphas[len-2] - a1 : 0;
        alphas[len-1] = alphas[len-1] > a2 ? alphas[len-1] - a2 : 0;
    } else {
        computeAlphaAboveLine(tempAlphas + uR - L, ur - (uR << 16), lr - (uR << 16),
                rDY, fullAlpha);
        for (int i = uR; i < lR; i++) {
            if (alphas[i - L] > tempAlphas[i - L]) {
                alphas[i - L] -= tempAlphas[i - L];
            } else {
                alphas[i - L] = 0;
            }
        }
    }

    if (isUsingMask) {
        for (int i=0; i<len; i++) {
            addAlpha(maskRow[L + i], alphas[i]);
        }
    } else {
        if (fullAlpha == 0xFF) { // Real blitter is faster than RunBasedAdditiveBlitter
            blitter->getRealBlitter()->blitAntiH(L, y, alphas, runs);
        } else {
            blitter->blitAntiH(L, y, alphas, len);
        }
    }

    if (len > kQuickLen) {
        delete [] alphas;
    }
}

static inline void blit_trapezoid_row(AdditiveBlitter* blitter, int y,
                               SkFixed ul, SkFixed ur, SkFixed ll, SkFixed lr,
                               SkFixed lDY, SkFixed rDY, SkAlpha fullAlpha,
                               SkAlpha* maskRow, bool isUsingMask) {
    SkASSERT(lDY >= 0 && rDY >= 0); // We should only send in the absolte value

    if (ul > ur) {
#ifdef SK_DEBUG
        SkDebugf("ul = %f > ur = %f!\n", SkFixedToFloat(ul), SkFixedToFloat(ur));
#endif
        return;
    }

    // Edge crosses. Approximate it. This should only happend due to precision limit,
    // so the approximation could be very coarse.
    if (ll > lr) {
#ifdef SK_DEBUG
        SkDebugf("approximate intersection: %d %f %f\n", y,
                 SkFixedToFloat(ll), SkFixedToFloat(lr));
#endif
        ll = lr = approximateIntersection(ul, ll, ur, lr);
    }

    if (ul == ur && ll == lr) {
        return; // empty trapzoid
    }

    // We're going to use the left line ul-ll and the rite line ur-lr
    // to exclude the area that's not covered by the path.
    // Swapping (ul, ll) or (ur, lr) won't affect that exclusion
    // so we'll do that for simplicity.
    if (ul > ll) { SkTSwap(ul, ll); }
    if (ur > lr) { SkTSwap(ur, lr); }

    SkFixed joinLeft = SkFixedCeilToFixed(ll);
    SkFixed joinRite = SkFixedFloorToFixed(ur);
    if (joinLeft <= joinRite) { // There's a rect from joinLeft to joinRite that we can blit
        if (ul < joinLeft) {
            int len = SkFixedCeilToInt(joinLeft - ul);
            if (len == 1) {
                SkAlpha alpha = trapezoidToAlpha(joinLeft - ul, joinLeft - ll);
                blit_single_alpha(blitter, y, ul >> 16, alpha, fullAlpha, maskRow, isUsingMask);
            } else if (len == 2) {
                SkFixed first = joinLeft - SK_Fixed1 - ul;
                SkFixed second = ll - ul - first;
                SkAlpha a1 = partialTriangleToAlpha(first, lDY);
                SkAlpha a2 = fullAlpha - partialTriangleToAlpha(second, lDY);
                blit_two_alphas(blitter, y, ul >> 16, a1, a2, fullAlpha, maskRow, isUsingMask);
            } else {
                blit_aaa_trapezoid_row(blitter, y, ul, joinLeft, ll, joinLeft, lDY, SK_MaxS32,
                                       fullAlpha, maskRow, isUsingMask);
            }
        }
        // SkAAClip requires that we blit from left to right.
        // Hence we must blit [ul, joinLeft] before blitting [joinLeft, joinRite]
        if (joinLeft < joinRite) {
            blit_full_alpha(blitter, y, SkFixedFloorToInt(joinLeft),
                            SkFixedFloorToInt(joinRite - joinLeft),
                            fullAlpha, maskRow, isUsingMask);
        }
        if (lr > joinRite) {
            int len = SkFixedCeilToInt(lr - joinRite);
            if (len == 1) {
                SkAlpha alpha = trapezoidToAlpha(ur - joinRite, lr - joinRite);
                blit_single_alpha(blitter, y, joinRite >> 16, alpha, fullAlpha, maskRow,
                                  isUsingMask);
            } else if (len == 2) {
                SkFixed first = joinRite + SK_Fixed1 - ur;
                SkFixed second = lr - ur - first;
                SkAlpha a1 = fullAlpha - partialTriangleToAlpha(first, rDY);
                SkAlpha a2 = partialTriangleToAlpha(second, rDY);
                blit_two_alphas(blitter, y, joinRite >> 16, a1, a2, fullAlpha, maskRow,
                                isUsingMask);
            } else {
                blit_aaa_trapezoid_row(blitter, y, joinRite, ur, joinRite, lr, SK_MaxS32, rDY,
                                       fullAlpha, maskRow, isUsingMask);
            }
        }
    } else {
        blit_aaa_trapezoid_row(blitter, y, ul, ur, ll, lr, lDY, rDY, fullAlpha, maskRow,
                               isUsingMask);
    }
}

///////////////////////////////////////////////////////////////////////////////

static bool operator<(const SkAnalyticEdge& a, const SkAnalyticEdge& b) {
    int valuea = a.fUpperY;
    int valueb = b.fUpperY;

    if (valuea == valueb) {
        valuea = a.fX;
        valueb = b.fX;
    }

    if (valuea == valueb) {
        valuea = a.fDX;
        valueb = b.fDX;
    }

    return valuea < valueb;
}

static SkAnalyticEdge* sort_edges(SkAnalyticEdge* list[], int count, SkAnalyticEdge** last) {
    SkTQSort(list, list + count - 1);

    // now make the edges linked in sorted order
    for (int i = 1; i < count; i++) {
        list[i - 1]->fNext = list[i];
        list[i]->fPrev = list[i - 1];
    }

    *last = list[count - 1];
    return list[0];
}

#ifdef SK_DEBUG
    static void validate_sort(const SkAnalyticEdge* edge) {
        SkFixed y = SkIntToFixed(-32768);

        while (edge->fUpperY != SK_MaxS32) {
            edge->validate();
            SkASSERT(y <= edge->fUpperY);

            y = edge->fUpperY;
            edge = (SkAnalyticEdge*)edge->fNext;
        }
    }
#else
    #define validate_sort(edge)
#endif

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

// For an edge, we consider it smooth if the Dx doesn't change much, and Dy is large enough
// For curves that are updating, the Dx is not changing much if fQDx/fCDx and fQDy/fCDy are
// relatively large compared to fQDDx/QCDDx and fQDDy/fCDDy
static inline bool isSmoothEnough(SkAnalyticEdge* thisEdge, SkAnalyticEdge* nextEdge, int stop_y) {
    if (thisEdge->fCurveCount < 0) {
        const SkCubicEdge& cEdge = static_cast<SkAnalyticCubicEdge*>(thisEdge)->fCEdge;
        int ddshift = cEdge.fCurveShift;
        return SkAbs32(cEdge.fCDx) >> 1 >= SkAbs32(cEdge.fCDDx) >> ddshift &&
                SkAbs32(cEdge.fCDy) >> 1 >= SkAbs32(cEdge.fCDDy) >> ddshift &&
                // current Dy is (fCDy - (fCDDy >> ddshift)) >> dshift
                (cEdge.fCDy - (cEdge.fCDDy >> ddshift)) >> cEdge.fCubicDShift >= SK_Fixed1;
    } else if (thisEdge->fCurveCount > 0) {
        const SkQuadraticEdge& qEdge = static_cast<SkAnalyticQuadraticEdge*>(thisEdge)->fQEdge;
        return SkAbs32(qEdge.fQDx) >> 1 >= SkAbs32(qEdge.fQDDx) &&
                SkAbs32(qEdge.fQDy) >> 1 >= SkAbs32(qEdge.fQDDy) &&
                // current Dy is (fQDy - fQDDy) >> shift
                (qEdge.fQDy - qEdge.fQDDy) >> qEdge.fCurveShift
                >= SK_Fixed1;
    }
    return SkAbs32(nextEdge->fDX - thisEdge->fDX) <= SK_Fixed1 && // DDx should be small
            nextEdge->fLowerY - nextEdge->fUpperY >= SK_Fixed1; // Dy should be large
}

// Check if the leftE and riteE are changing smoothly in terms of fDX.
// If yes, we can later skip the fractional y and directly jump to integer y.
static inline bool isSmoothEnough(SkAnalyticEdge* leftE, SkAnalyticEdge* riteE,
                           SkAnalyticEdge* currE, int stop_y) {
    if (currE->fUpperY >= stop_y << 16) {
        return false; // We're at the end so we won't skip anything
    }
    if (leftE->fLowerY + SK_Fixed1 < riteE->fLowerY) {
        return isSmoothEnough(leftE, currE, stop_y); // Only leftE is changing
    } else if (leftE->fLowerY > riteE->fLowerY + SK_Fixed1) {
        return isSmoothEnough(riteE, currE, stop_y); // Only riteE is changing
    }

    // Now both edges are changing, find the second next edge
    SkAnalyticEdge* nextCurrE = currE->fNext;
    if (nextCurrE->fUpperY >= stop_y << 16) { // Check if we're at the end
        return false;
    }
    if (*nextCurrE < *currE) {
        SkTSwap(currE, nextCurrE);
    }
    return isSmoothEnough(leftE, currE, stop_y) && isSmoothEnough(riteE, nextCurrE, stop_y);
}

static inline void aaa_walk_convex_edges(SkAnalyticEdge* prevHead, AdditiveBlitter* blitter,
                           int start_y, int stop_y, SkFixed leftBound, SkFixed riteBound,
                           bool isUsingMask) {
    validate_sort((SkAnalyticEdge*)prevHead->fNext);

    SkAnalyticEdge* leftE = (SkAnalyticEdge*) prevHead->fNext;
    SkAnalyticEdge* riteE = (SkAnalyticEdge*) leftE->fNext;
    SkAnalyticEdge* currE = (SkAnalyticEdge*) riteE->fNext;

    SkFixed y = SkTMax(leftE->fUpperY, riteE->fUpperY);

    #ifdef SK_DEBUG
    int frac_y_cnt = 0;
    int total_y_cnt = 0;
    #endif

    for (;;) {
        // We have to check fLowerY first because some edges might be alone (e.g., there's only
        // a left edge but no right edge in a given y scan line) due to precision limit.
        while (leftE->fLowerY <= y) { // Due to smooth jump, we may pass multiple short edges
            if (update_edge(leftE, y)) {
                if (SkFixedFloorToInt(currE->fUpperY) >= stop_y) {
                    goto END_WALK;
                }
                leftE = currE;
                currE = (SkAnalyticEdge*)currE->fNext;
            }
        }
        while (riteE->fLowerY <= y) { // Due to smooth jump, we may pass multiple short edges
            if (update_edge(riteE, y)) {
                if (SkFixedFloorToInt(currE->fUpperY) >= stop_y) {
                    goto END_WALK;
                }
                riteE = currE;
                currE = (SkAnalyticEdge*)currE->fNext;
            }
        }

        SkASSERT(leftE);
        SkASSERT(riteE);

        // check our bottom clip
        if (SkFixedFloorToInt(y) >= stop_y) {
            break;
        }

        SkASSERT(SkFixedFloorToInt(leftE->fUpperY) <= stop_y);
        SkASSERT(SkFixedFloorToInt(riteE->fUpperY) <= stop_y);

        leftE->goY(y);
        riteE->goY(y);

        if (leftE->fX > riteE->fX || (leftE->fX == riteE->fX &&
                                      leftE->fDX > riteE->fDX)) {
            SkTSwap(leftE, riteE);
        }

        SkFixed local_bot_fixed = SkMin32(leftE->fLowerY, riteE->fLowerY);
        if (isSmoothEnough(leftE, riteE, currE, stop_y)) {
            local_bot_fixed = SkFixedCeilToFixed(local_bot_fixed);
        }
        local_bot_fixed = SkMin32(local_bot_fixed, SkIntToFixed(stop_y));

        SkFixed left = SkTMax(leftBound, leftE->fX);
        SkFixed dLeft = leftE->fDX;
        SkFixed rite = SkTMin(riteBound, riteE->fX);
        SkFixed dRite = riteE->fDX;
        if (0 == (dLeft | dRite)) {
            int     fullLeft    = SkFixedCeilToInt(left);
            int     fullRite    = SkFixedFloorToInt(rite);
            SkFixed partialLeft = SkIntToFixed(fullLeft) - left;
            SkFixed partialRite = rite - SkIntToFixed(fullRite);
            int     fullTop     = SkFixedCeilToInt(y);
            int     fullBot     = SkFixedFloorToInt(local_bot_fixed);
            SkFixed partialTop  = SkIntToFixed(fullTop) - y;
            SkFixed partialBot  = local_bot_fixed - SkIntToFixed(fullBot);
            if (fullTop > fullBot) { // The rectangle is within one pixel height...
                partialTop -= (SK_Fixed1 - partialBot);
                partialBot = 0;
            }

            if (fullRite >= fullLeft) {
                if (partialTop > 0) { // blit first partial row
                    if (partialLeft > 0) {
                        blitter->blitAntiH(fullLeft - 1, fullTop - 1,
                                f2a(SkFixedMul_lowprec(partialTop, partialLeft)));
                    }
                    blitter->blitAntiH(fullLeft, fullTop - 1, fullRite - fullLeft,
                                       f2a(partialTop));
                    if (partialRite > 0) {
                        blitter->blitAntiH(fullRite, fullTop - 1,
                                f2a(SkFixedMul_lowprec(partialTop, partialRite)));
                    }
                    blitter->flush_if_y_changed(y, y + partialTop);
                }

                // Blit all full-height rows from fullTop to fullBot
                if (fullBot > fullTop &&
                        // SkAAClip cannot handle the empty rect so check the non-emptiness here
                        // (bug chromium:662800)
                        (fullRite > fullLeft || f2a(partialLeft) > 0 || f2a(partialRite) > 0)) {
                    blitter->getRealBlitter()->blitAntiRect(fullLeft - 1, fullTop,
                                                            fullRite - fullLeft, fullBot - fullTop,
                                                            f2a(partialLeft), f2a(partialRite));
                }

                if (partialBot > 0) { // blit last partial row
                    if (partialLeft > 0) {
                        blitter->blitAntiH(fullLeft - 1, fullBot,
                                           f2a(SkFixedMul_lowprec(partialBot, partialLeft)));
                    }
                    blitter->blitAntiH(fullLeft, fullBot, fullRite - fullLeft, f2a(partialBot));
                    if (partialRite > 0) {
                        blitter->blitAntiH(fullRite, fullBot,
                                           f2a(SkFixedMul_lowprec(partialBot, partialRite)));
                    }
                }
            } else { // left and rite are within the same pixel
                if (partialTop > 0) {
                    blitter->getRealBlitter()->blitV(fullLeft - 1, fullTop - 1, 1,
                            f2a(SkFixedMul_lowprec(partialTop, rite - left)));
                    blitter->flush_if_y_changed(y, y + partialTop);
                }
                if (fullBot > fullTop) {
                    blitter->getRealBlitter()->blitV(fullLeft - 1, fullTop, fullBot - fullTop,
                            f2a(rite - left));
                }
                if (partialBot > 0) {
                    blitter->getRealBlitter()->blitV(fullLeft - 1, fullBot, 1,
                            f2a(SkFixedMul_lowprec(partialBot, rite - left)));
                }
            }

            y = local_bot_fixed;
        } else {
            // The following constant are used to snap X
            // We snap X mainly for speedup (no tiny triangle) and
            // avoiding edge cases caused by precision errors
            const SkFixed kSnapDigit = SK_Fixed1 >> 4;
            const SkFixed kSnapHalf = kSnapDigit >> 1;
            const SkFixed kSnapMask = (-1 ^ (kSnapDigit - 1));
            left += kSnapHalf; rite += kSnapHalf; // For fast rounding

            // Number of blit_trapezoid_row calls we'll have
            int count = SkFixedCeilToInt(local_bot_fixed) - SkFixedFloorToInt(y);
            #ifdef SK_DEBUG
            total_y_cnt += count;
            frac_y_cnt += ((int)(y & 0xFFFF0000) != y);
            if ((int)(y & 0xFFFF0000) != y) {
                // SkDebugf("frac_y = %f\n", SkFixedToFloat(y));
            }
            #endif

            // If we're using mask blitter, we advance the mask row in this function
            // to save some "if" condition checks.
            SkAlpha* maskRow = nullptr;
            if (isUsingMask) {
                maskRow = static_cast<MaskAdditiveBlitter*>(blitter)->getRow(y >> 16);
            }

            // Instead of writing one loop that handles both partial-row blit_trapezoid_row
            // and full-row trapezoid_row together, we use the following 3-stage flow to
            // handle partial-row blit and full-row blit separately. It will save us much time
            // on changing y, left, and rite.
            if (count > 1) {
                if ((int)(y & 0xFFFF0000) != y) { // There's a partial-row on the top
                    count--;
                    SkFixed nextY = SkFixedCeilToFixed(y + 1);
                    SkFixed dY = nextY - y;
                    SkFixed nextLeft = left + SkFixedMul_lowprec(dLeft, dY);
                    SkFixed nextRite = rite + SkFixedMul_lowprec(dRite, dY);
                    SkASSERT((left & kSnapMask) >= leftBound && (rite & kSnapMask) <= riteBound &&
                            (nextLeft & kSnapMask) >= leftBound &&
                            (nextRite & kSnapMask) <= riteBound);
                    blit_trapezoid_row(blitter, y >> 16, left & kSnapMask, rite & kSnapMask,
                            nextLeft & kSnapMask, nextRite & kSnapMask, leftE->fDY, riteE->fDY,
                            getPartialAlpha(0xFF, dY), maskRow, isUsingMask);
                    blitter->flush_if_y_changed(y, nextY);
                    left = nextLeft; rite = nextRite; y = nextY;
                }

                while (count > 1) { // Full rows in the middle
                    count--;
                    if (isUsingMask) {
                        maskRow = static_cast<MaskAdditiveBlitter*>(blitter)->getRow(y >> 16);
                    }
                    SkFixed nextY = y + SK_Fixed1, nextLeft = left + dLeft, nextRite = rite + dRite;
                    SkASSERT((left & kSnapMask) >= leftBound && (rite & kSnapMask) <= riteBound &&
                            (nextLeft & kSnapMask) >= leftBound &&
                            (nextRite & kSnapMask) <= riteBound);
                    blit_trapezoid_row(blitter, y >> 16, left & kSnapMask, rite & kSnapMask,
                            nextLeft & kSnapMask, nextRite & kSnapMask,
                            leftE->fDY, riteE->fDY, 0xFF, maskRow, isUsingMask);
                    blitter->flush_if_y_changed(y, nextY);
                    left = nextLeft; rite = nextRite; y = nextY;
                }
            }

            if (isUsingMask) {
                maskRow = static_cast<MaskAdditiveBlitter*>(blitter)->getRow(y >> 16);
            }

            SkFixed dY = local_bot_fixed - y; // partial-row on the bottom
            SkASSERT(dY <= SK_Fixed1);
            // Smooth jumping to integer y may make the last nextLeft/nextRite out of bound.
            // Take them back into the bound here.
            // Note that we substract kSnapHalf later so we have to add them to leftBound/riteBound
            SkFixed nextLeft = SkTMax(left + SkFixedMul_lowprec(dLeft, dY), leftBound + kSnapHalf);
            SkFixed nextRite = SkTMin(rite + SkFixedMul_lowprec(dRite, dY), riteBound + kSnapHalf);
            SkASSERT((left & kSnapMask) >= leftBound && (rite & kSnapMask) <= riteBound &&
                    (nextLeft & kSnapMask) >= leftBound && (nextRite & kSnapMask) <= riteBound);
            blit_trapezoid_row(blitter, y >> 16, left & kSnapMask, rite & kSnapMask,
                    nextLeft & kSnapMask, nextRite & kSnapMask, leftE->fDY, riteE->fDY,
                    getPartialAlpha(0xFF, dY), maskRow, isUsingMask);
            blitter->flush_if_y_changed(y, local_bot_fixed);
            left = nextLeft; rite = nextRite; y = local_bot_fixed;
            left -= kSnapHalf; rite -= kSnapHalf;
        }

        leftE->fX = left;
        riteE->fX = rite;
        leftE->fY = riteE->fY = y;
    }

END_WALK:
    ;
    #ifdef SK_DEBUG
    // SkDebugf("frac_y_cnt = %d, total_y_cnt = %d\n", frac_y_cnt, total_y_cnt);
    #endif
}

void aaa_fill_path(const SkPath& path, const SkIRect& clipRect, AdditiveBlitter* blitter,
                   int start_y, int stop_y, bool pathContainedInClip, bool isUsingMask,
                   bool forceRLE) { // forceRLE implies that SkAAClip is calling us
    SkASSERT(blitter);

    // we only implemented the convex shapes yet
    SkASSERT(!path.isInverseFillType() && path.isConvex());

    SkEdgeBuilder   builder;

    // If we're convex, then we need both edges, even the right edge is past the clip
    const bool canCullToTheRight = !path.isConvex();

    SkASSERT(gSkUseAnalyticAA.load());
    const SkIRect* builderClip = pathContainedInClip ? nullptr : &clipRect;
    int count = builder.build(path, builderClip, 0, canCullToTheRight, true);
    SkASSERT(count >= 0);

    SkAnalyticEdge** list = (SkAnalyticEdge**)builder.analyticEdgeList();

    SkIRect rect = clipRect;
    if (0 == count) {
        if (path.isInverseFillType()) {
            /*
             *  Since we are in inverse-fill, our caller has already drawn above
             *  our top (start_y) and will draw below our bottom (stop_y). Thus
             *  we need to restrict our drawing to the intersection of the clip
             *  and those two limits.
             */
            if (rect.fTop < start_y) {
                rect.fTop = start_y;
            }
            if (rect.fBottom > stop_y) {
                rect.fBottom = stop_y;
            }
            if (!rect.isEmpty()) {
                blitter->blitRect(rect.fLeft, rect.fTop, rect.width(), rect.height());
            }
        }
        return;
    }

    SkAnalyticEdge headEdge, tailEdge, *last;
    // this returns the first and last edge after they're sorted into a dlink list
    SkAnalyticEdge* edge = sort_edges(list, count, &last);

    headEdge.fPrev = nullptr;
    headEdge.fNext = edge;
    headEdge.fUpperY = headEdge.fLowerY = SK_MinS32;
    headEdge.fX = SK_MinS32;
    headEdge.fDX = 0;
    headEdge.fDY = SK_MaxS32;
    headEdge.fUpperX = SK_MinS32;
    edge->fPrev = &headEdge;

    tailEdge.fPrev = last;
    tailEdge.fNext = nullptr;
    tailEdge.fUpperY = tailEdge.fLowerY = SK_MaxS32;
    headEdge.fX = SK_MaxS32;
    headEdge.fDX = 0;
    headEdge.fDY = SK_MaxS32;
    headEdge.fUpperX = SK_MaxS32;
    last->fNext = &tailEdge;

    // now edge is the head of the sorted linklist

    if (!pathContainedInClip && start_y < clipRect.fTop) {
        start_y = clipRect.fTop;
    }
    if (!pathContainedInClip && stop_y > clipRect.fBottom) {
        stop_y = clipRect.fBottom;
    }

    if (!path.isInverseFillType() && path.isConvex()) {
        SkASSERT(count >= 2);   // convex walker does not handle missing right edges
        SkFixed leftBound = SkIntToFixed(rect.fLeft);
        SkFixed rightBound = SkIntToFixed(rect.fRight);
        if (isUsingMask) {
            // If we're using mask, then we have to limit the bound within the path bounds.
            // Otherwise, the edge drift may access an invalid address inside the mask.
            SkIRect ir;
            path.getBounds().roundOut(&ir);
            leftBound = SkTMax(leftBound, SkIntToFixed(ir.fLeft));
            rightBound = SkTMin(rightBound, SkIntToFixed(ir.fRight));
        }
        aaa_walk_convex_edges(&headEdge, blitter, start_y, stop_y,
                              leftBound, rightBound, isUsingMask);
    } else {
        SkFAIL("Concave AAA is not yet implemented!");
    }
}

///////////////////////////////////////////////////////////////////////////////

static int overflows_short_shift(int value, int shift) {
    const int s = 16 + shift;
    return (SkLeftShift(value, s) >> s) - value;
}

/**
  Would any of the coordinates of this rectangle not fit in a short,
  when left-shifted by shift?
*/
static int rect_overflows_short_shift(SkIRect rect, int shift) {
    SkASSERT(!overflows_short_shift(8191, 2));
    SkASSERT(overflows_short_shift(8192, 2));
    SkASSERT(!overflows_short_shift(32767, 0));
    SkASSERT(overflows_short_shift(32768, 0));

    // Since we expect these to succeed, we bit-or together
    // for a tiny extra bit of speed.
    return overflows_short_shift(rect.fLeft, 2) |
           overflows_short_shift(rect.fRight, 2) |
           overflows_short_shift(rect.fTop, 2) |
           overflows_short_shift(rect.fBottom, 2);
}

static bool fitsInsideLimit(const SkRect& r, SkScalar max) {
    const SkScalar min = -max;
    return  r.fLeft > min && r.fTop > min &&
            r.fRight < max && r.fBottom < max;
}

static bool safeRoundOut(const SkRect& src, SkIRect* dst, int32_t maxInt) {
    const SkScalar maxScalar = SkIntToScalar(maxInt);

    if (fitsInsideLimit(src, maxScalar)) {
        src.roundOut(dst);
        return true;
    }
    return false;
}

void SkScan::AAAFillPath(const SkPath& path, const SkRegion& origClip, SkBlitter* blitter,
                         bool forceRLE) {
    if (origClip.isEmpty()) {
        return;
    }
    if (path.isInverseFillType() || !path.isConvex()) {
        // Fall back as we only implemented the algorithm for convex shapes yet.
        SkScan::AntiFillPath(path, origClip, blitter, forceRLE);
        return;
    }

    const bool isInverse = path.isInverseFillType();
    SkIRect ir;
    if (!safeRoundOut(path.getBounds(), &ir, SK_MaxS32 >> 2)) {
        return;
    }
    if (ir.isEmpty()) {
        if (isInverse) {
            blitter->blitRegion(origClip);
        }
        return;
    }

    SkIRect clippedIR;
    if (isInverse) {
       // If the path is an inverse fill, it's going to fill the entire
       // clip, and we care whether the entire clip exceeds our limits.
       clippedIR = origClip.getBounds();
    } else {
       if (!clippedIR.intersect(ir, origClip.getBounds())) {
           return;
       }
    }
    // If the intersection of the path bounds and the clip bounds
    // will overflow 32767 when << by 2, our SkFixed will overflow,
    // so draw without antialiasing.
    if (rect_overflows_short_shift(clippedIR, 2)) {
        SkScan::FillPath(path, origClip, blitter);
        return;
    }

    // Our antialiasing can't handle a clip larger than 32767, so we restrict
    // the clip to that limit here. (the runs[] uses int16_t for its index).
    //
    // A more general solution (one that could also eliminate the need to
    // disable aa based on ir bounds (see overflows_short_shift) would be
    // to tile the clip/target...
    SkRegion tmpClipStorage;
    const SkRegion* clipRgn = &origClip;
    {
        static const int32_t kMaxClipCoord = 32767;
        const SkIRect& bounds = origClip.getBounds();
        if (bounds.fRight > kMaxClipCoord || bounds.fBottom > kMaxClipCoord) {
            SkIRect limit = { 0, 0, kMaxClipCoord, kMaxClipCoord };
            tmpClipStorage.op(origClip, limit, SkRegion::kIntersect_Op);
            clipRgn = &tmpClipStorage;
        }
    }
    // for here down, use clipRgn, not origClip

    SkScanClipper   clipper(blitter, clipRgn, ir);
    const SkIRect*  clipRect = clipper.getClipRect();

    if (clipper.getBlitter() == nullptr) { // clipped out
        if (isInverse) {
            blitter->blitRegion(*clipRgn);
        }
        return;
    }

    // now use the (possibly wrapped) blitter
    blitter = clipper.getBlitter();

    if (isInverse) {
        // Currently, we use the old path to render the inverse path,
        // so we don't need this.
        // sk_blit_above(blitter, ir, *clipRgn);
    }

    SkASSERT(SkIntToScalar(ir.fTop) <= path.getBounds().fTop);

    if (MaskAdditiveBlitter::canHandleRect(ir) && !isInverse && !forceRLE) {
        MaskAdditiveBlitter additiveBlitter(blitter, ir, *clipRgn, isInverse);
        aaa_fill_path(path, clipRgn->getBounds(), &additiveBlitter, ir.fTop, ir.fBottom,
                clipRect == nullptr, true, forceRLE);
    } else {
        RunBasedAdditiveBlitter additiveBlitter(blitter, ir, *clipRgn, isInverse);
        aaa_fill_path(path, clipRgn->getBounds(), &additiveBlitter, ir.fTop, ir.fBottom,
                clipRect == nullptr, false, forceRLE);
    }

    if (isInverse) {
        // Currently, we use the old path to render the inverse path,
        // so we don't need this.
        // sk_blit_below(blitter, ir, *clipRgn);
    }
}

// This almost copies SkScan::AntiFillPath
void SkScan::AAAFillPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    if (clip.isEmpty()) {
        return;
    }

    if (clip.isBW()) {
        AAAFillPath(path, clip.bwRgn(), blitter);
    } else {
        SkRegion        tmp;
        SkAAClipBlitter aaBlitter;

        tmp.setRect(clip.getBounds());
        aaBlitter.init(blitter, &clip.aaRgn());
        AAAFillPath(path, tmp, &aaBlitter, true);
    }
}
