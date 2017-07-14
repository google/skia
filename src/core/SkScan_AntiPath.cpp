/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScanPriv.h"
#include "SkPath.h"
#include "SkMatrix.h"
#include "SkBlitter.h"
#include "SkRegion.h"
#include "SkAntiRun.h"

#define SHIFT   2
#define SCALE   (1 << SHIFT)
#define MASK    (SCALE - 1)

/** @file
    We have two techniques for capturing the output of the supersampler:
    - SUPERMASK, which records a large mask-bitmap
        this is often faster for small, complex objects
    - RLE, which records a rle-encoded scanline
        this is often faster for large objects with big spans

    These blitters use two coordinate systems:
    - destination coordinates, scale equal to the output - often
        abbreviated with 'i' or 'I' in variable names
    - supersampled coordinates, scale equal to the output * SCALE
 */

//#define FORCE_SUPERMASK
//#define FORCE_RLE

///////////////////////////////////////////////////////////////////////////////

/// Base class for a single-pass supersampled blitter.
class BaseSuperBlitter : public SkBlitter {
public:
    BaseSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                     const SkRegion& clip, bool isInverse);

    /// Must be explicitly defined on subclasses.
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[],
                           const int16_t runs[]) override {
        SkDEBUGFAIL("How did I get here?");
    }
    /// May not be called on BaseSuperBlitter because it blits out of order.
    void blitV(int x, int y, int height, SkAlpha alpha) override {
        SkDEBUGFAIL("How did I get here?");
    }

protected:
    SkBlitter*  fRealBlitter;
    /// Current y coordinate, in destination coordinates.
    int         fCurrIY;
    /// Widest row of region to be blitted, in destination coordinates.
    int         fWidth;
    /// Leftmost x coordinate in any row, in destination coordinates.
    int         fLeft;
    /// Leftmost x coordinate in any row, in supersampled coordinates.
    int         fSuperLeft;

    SkDEBUGCODE(int fCurrX;)
    /// Current y coordinate in supersampled coordinates.
    int fCurrY;
    /// Initial y coordinate (top of bounds).
    int fTop;

    SkIRect fSectBounds;
};

BaseSuperBlitter::BaseSuperBlitter(SkBlitter* realBlit, const SkIRect& ir, const SkRegion& clip,
                                   bool isInverse) {
    fRealBlitter = realBlit;

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
    fSuperLeft = SkLeftShift(left, SHIFT);
    fWidth = right - left;
    fTop = sectBounds.top();
    fCurrIY = fTop - 1;
    fCurrY = SkLeftShift(fTop, SHIFT) - 1;

    SkDEBUGCODE(fCurrX = -1;)
}

/// Run-length-encoded supersampling antialiased blitter.
class SuperBlitter : public BaseSuperBlitter {
public:
    SuperBlitter(SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip, bool isInverse);

    ~SuperBlitter() override {
        this->flush();
    }

    /// Once fRuns contains a complete supersampled row, flush() blits
    /// it out through the wrapped blitter.
    void flush();

    /// Blits a row of pixels, with location and width specified
    /// in supersampled coordinates.
    void blitH(int x, int y, int width) override;
    /// Blits a rectangle of pixels, with location and size specified
    /// in supersampled coordinates.
    void blitRect(int x, int y, int width, int height) override;

private:
    // The next three variables are used to track a circular buffer that
    // contains the values used in SkAlphaRuns. These variables should only
    // ever be updated in advanceRuns(), and fRuns should always point to
    // a valid SkAlphaRuns...
    int         fRunsToBuffer;
    void*       fRunsBuffer;
    int         fCurrentRun;
    SkAlphaRuns fRuns;

    // extra one to store the zero at the end
    int getRunsSz() const { return (fWidth + 1 + (fWidth + 2)/2) * sizeof(int16_t); }

    // This function updates the fRuns variable to point to the next buffer space
    // with adequate storage for a SkAlphaRuns. It mostly just advances fCurrentRun
    // and resets fRuns to point to an empty scanline.
    void advanceRuns() {
        const size_t kRunsSz = this->getRunsSz();
        fCurrentRun = (fCurrentRun + 1) % fRunsToBuffer;
        fRuns.fRuns = reinterpret_cast<int16_t*>(
            reinterpret_cast<uint8_t*>(fRunsBuffer) + fCurrentRun * kRunsSz);
        fRuns.fAlpha = reinterpret_cast<SkAlpha*>(fRuns.fRuns + fWidth + 1);
        fRuns.reset(fWidth);
    }

    int         fOffsetX;
};

SuperBlitter::SuperBlitter(SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip,
                           bool isInverse)
        : BaseSuperBlitter(realBlitter, ir, clip, isInverse)
{
    fRunsToBuffer = realBlitter->requestRowsPreserved();
    fRunsBuffer = realBlitter->allocBlitMemory(fRunsToBuffer * this->getRunsSz());
    fCurrentRun = -1;

    this->advanceRuns();

    fOffsetX = 0;
}

void SuperBlitter::flush() {
    if (fCurrIY >= fTop) {

        SkASSERT(fCurrentRun < fRunsToBuffer);
        if (!fRuns.empty()) {
            // SkDEBUGCODE(fRuns.dump();)
            fRealBlitter->blitAntiH(fLeft, fCurrIY, fRuns.fAlpha, fRuns.fRuns);
            this->advanceRuns();
            fOffsetX = 0;
        }

        fCurrIY = fTop - 1;
        SkDEBUGCODE(fCurrX = -1;)
    }
}

/** coverage_to_partial_alpha() is being used by SkAlphaRuns, which
    *accumulates* SCALE pixels worth of "alpha" in [0,(256/SCALE)]
    to produce a final value in [0, 255] and handles clamping 256->255
    itself, with the same (alpha - (alpha >> 8)) correction as
    coverage_to_exact_alpha().
*/
static inline int coverage_to_partial_alpha(int aa) {
    aa <<= 8 - 2*SHIFT;
    return aa;
}

/** coverage_to_exact_alpha() is being used by our blitter, which wants
    a final value in [0, 255].
*/
static inline int coverage_to_exact_alpha(int aa) {
    int alpha = (256 >> SHIFT) * aa;
    // clamp 256->255
    return alpha - (alpha >> 8);
}

void SuperBlitter::blitH(int x, int y, int width) {
    SkASSERT(width > 0);

    int iy = y >> SHIFT;
    SkASSERT(iy >= fCurrIY);

    x -= fSuperLeft;
    // hack, until I figure out why my cubics (I think) go beyond the bounds
    if (x < 0) {
        width += x;
        x = 0;
    }

#ifdef SK_DEBUG
    SkASSERT(y != fCurrY || x >= fCurrX);
#endif
    SkASSERT(y >= fCurrY);
    if (fCurrY != y) {
        fOffsetX = 0;
        fCurrY = y;
    }

    if (iy != fCurrIY) {  // new scanline
        this->flush();
        fCurrIY = iy;
    }

    int start = x;
    int stop = x + width;

    SkASSERT(start >= 0 && stop > start);
    // integer-pixel-aligned ends of blit, rounded out
    int fb = start & MASK;
    int fe = stop & MASK;
    int n = (stop >> SHIFT) - (start >> SHIFT) - 1;

    if (n < 0) {
        fb = fe - fb;
        n = 0;
        fe = 0;
    } else {
        if (fb == 0) {
            n += 1;
        } else {
            fb = SCALE - fb;
        }
    }

    fOffsetX = fRuns.add(x >> SHIFT, coverage_to_partial_alpha(fb),
                         n, coverage_to_partial_alpha(fe),
                         (1 << (8 - SHIFT)) - (((y & MASK) + 1) >> SHIFT),
                         fOffsetX);

#ifdef SK_DEBUG
    fRuns.assertValid(y & MASK, (1 << (8 - SHIFT)));
    fCurrX = x + width;
#endif
}

#if 0 // UNUSED
static void set_left_rite_runs(SkAlphaRuns& runs, int ileft, U8CPU leftA,
                               int n, U8CPU riteA) {
    SkASSERT(leftA <= 0xFF);
    SkASSERT(riteA <= 0xFF);

    int16_t* run = runs.fRuns;
    uint8_t* aa = runs.fAlpha;

    if (ileft > 0) {
        run[0] = ileft;
        aa[0] = 0;
        run += ileft;
        aa += ileft;
    }

    SkASSERT(leftA < 0xFF);
    if (leftA > 0) {
        *run++ = 1;
        *aa++ = leftA;
    }

    if (n > 0) {
        run[0] = n;
        aa[0] = 0xFF;
        run += n;
        aa += n;
    }

    SkASSERT(riteA < 0xFF);
    if (riteA > 0) {
        *run++ = 1;
        *aa++ = riteA;
    }
    run[0] = 0;
}
#endif

void SuperBlitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(width > 0);
    SkASSERT(height > 0);

    // blit leading rows
    while ((y & MASK)) {
        this->blitH(x, y++, width);
        if (--height <= 0) {
            return;
        }
    }
    SkASSERT(height > 0);

    // Since this is a rect, instead of blitting supersampled rows one at a
    // time and then resolving to the destination canvas, we can blit
    // directly to the destintion canvas one row per SCALE supersampled rows.
    int start_y = y >> SHIFT;
    int stop_y = (y + height) >> SHIFT;
    int count = stop_y - start_y;
    if (count > 0) {
        y += count << SHIFT;
        height -= count << SHIFT;

        // save original X for our tail blitH() loop at the bottom
        int origX = x;

        x -= fSuperLeft;
        // hack, until I figure out why my cubics (I think) go beyond the bounds
        if (x < 0) {
            width += x;
            x = 0;
        }

        // There is always a left column, a middle, and a right column.
        // ileft is the destination x of the first pixel of the entire rect.
        // xleft is (SCALE - # of covered supersampled pixels) in that
        // destination pixel.
        int ileft = x >> SHIFT;
        int xleft = x & MASK;
        // irite is the destination x of the last pixel of the OPAQUE section.
        // xrite is the number of supersampled pixels extending beyond irite;
        // xrite/SCALE should give us alpha.
        int irite = (x + width) >> SHIFT;
        int xrite = (x + width) & MASK;
        if (!xrite) {
            xrite = SCALE;
            irite--;
        }

        // Need to call flush() to clean up pending draws before we
        // even consider blitV(), since otherwise it can look nonmonotonic.
        SkASSERT(start_y > fCurrIY);
        this->flush();

        int n = irite - ileft - 1;
        if (n < 0) {
            // If n < 0, we'll only have a single partially-transparent column
            // of pixels to render.
            xleft = xrite - xleft;
            SkASSERT(xleft <= SCALE);
            SkASSERT(xleft > 0);
            fRealBlitter->blitV(ileft + fLeft, start_y, count,
                coverage_to_exact_alpha(xleft));
        } else {
            // With n = 0, we have two possibly-transparent columns of pixels
            // to render; with n > 0, we have opaque columns between them.

            xleft = SCALE - xleft;

            // Using coverage_to_exact_alpha is not consistent with blitH()
            const int coverageL = coverage_to_exact_alpha(xleft);
            const int coverageR = coverage_to_exact_alpha(xrite);

            SkASSERT(coverageL > 0 || n > 0 || coverageR > 0);
            SkASSERT((coverageL != 0) + n + (coverageR != 0) <= fWidth);

            fRealBlitter->blitAntiRect(ileft + fLeft, start_y, n, count,
                                       coverageL, coverageR);
        }

        // preamble for our next call to blitH()
        fCurrIY = stop_y - 1;
        fOffsetX = 0;
        fCurrY = y - 1;
        fRuns.reset(fWidth);
        x = origX;
    }

    // catch any remaining few rows
    SkASSERT(height <= MASK);
    while (--height >= 0) {
        this->blitH(x, y++, width);
    }
}

///////////////////////////////////////////////////////////////////////////////

/// Masked supersampling antialiased blitter.
class MaskSuperBlitter : public BaseSuperBlitter {
public:
    MaskSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir, const SkRegion&, bool isInverse);
    ~MaskSuperBlitter() override {
        fRealBlitter->blitMask(fMask, fClipRect);
    }

    void blitH(int x, int y, int width) override;

    static bool CanHandleRect(const SkIRect& bounds) {
#ifdef FORCE_RLE
        return false;
#endif
        int width = bounds.width();
        int64_t rb = SkAlign4(width);
        // use 64bits to detect overflow
        int64_t storage = rb * bounds.height();

        return (width <= MaskSuperBlitter::kMAX_WIDTH) &&
               (storage <= MaskSuperBlitter::kMAX_STORAGE);
    }

private:
    enum {
#ifdef FORCE_SUPERMASK
        kMAX_WIDTH = 2048,
        kMAX_STORAGE = 1024 * 1024 * 2
#else
        kMAX_WIDTH = 32,    // so we don't try to do very wide things, where the RLE blitter would be faster
        kMAX_STORAGE = 1024
#endif
    };

    SkMask      fMask;
    SkIRect     fClipRect;
    // we add 1 because add_aa_span can write (unchanged) 1 extra byte at the end, rather than
    // perform a test to see if stopAlpha != 0
    uint32_t    fStorage[(kMAX_STORAGE >> 2) + 1];
};

MaskSuperBlitter::MaskSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir, const SkRegion& clip,
                                   bool isInverse)
    : BaseSuperBlitter(realBlitter, ir, clip, isInverse)
{
    SkASSERT(CanHandleRect(ir));
    SkASSERT(!isInverse);

    fMask.fImage    = (uint8_t*)fStorage;
    fMask.fBounds   = ir;
    fMask.fRowBytes = ir.width();
    fMask.fFormat   = SkMask::kA8_Format;

    fClipRect = ir;
    if (!fClipRect.intersect(clip.getBounds())) {
        SkASSERT(0);
        fClipRect.setEmpty();
    }

    // For valgrind, write 1 extra byte at the end so we don't read
    // uninitialized memory. See comment in add_aa_span and fStorage[].
    memset(fStorage, 0, fMask.fBounds.height() * fMask.fRowBytes + 1);
}

static void add_aa_span(uint8_t* alpha, U8CPU startAlpha) {
    /*  I should be able to just add alpha[x] + startAlpha.
        However, if the trailing edge of the previous span and the leading
        edge of the current span round to the same super-sampled x value,
        I might overflow to 256 with this add, hence the funny subtract.
    */
    unsigned tmp = *alpha + startAlpha;
    SkASSERT(tmp <= 256);
    *alpha = SkToU8(tmp - (tmp >> 8));
}

static inline uint32_t quadplicate_byte(U8CPU value) {
    uint32_t pair = (value << 8) | value;
    return (pair << 16) | pair;
}

// Perform this tricky subtract, to avoid overflowing to 256. Our caller should
// only ever call us with at most enough to hit 256 (never larger), so it is
// enough to just subtract the high-bit. Actually clamping with a branch would
// be slower (e.g. if (tmp > 255) tmp = 255;)
//
static inline void saturated_add(uint8_t* ptr, U8CPU add) {
    unsigned tmp = *ptr + add;
    SkASSERT(tmp <= 256);
    *ptr = SkToU8(tmp - (tmp >> 8));
}

// minimum count before we want to setup an inner loop, adding 4-at-a-time
#define MIN_COUNT_FOR_QUAD_LOOP  16

static void add_aa_span(uint8_t* alpha, U8CPU startAlpha, int middleCount,
                        U8CPU stopAlpha, U8CPU maxValue) {
    SkASSERT(middleCount >= 0);

    saturated_add(alpha, startAlpha);
    alpha += 1;

    if (middleCount >= MIN_COUNT_FOR_QUAD_LOOP) {
        // loop until we're quad-byte aligned
        while (SkTCast<intptr_t>(alpha) & 0x3) {
            alpha[0] = SkToU8(alpha[0] + maxValue);
            alpha += 1;
            middleCount -= 1;
        }

        int bigCount = middleCount >> 2;
        uint32_t* qptr = reinterpret_cast<uint32_t*>(alpha);
        uint32_t qval = quadplicate_byte(maxValue);
        do {
            *qptr++ += qval;
        } while (--bigCount > 0);

        middleCount &= 3;
        alpha = reinterpret_cast<uint8_t*> (qptr);
        // fall through to the following while-loop
    }

    while (--middleCount >= 0) {
        alpha[0] = SkToU8(alpha[0] + maxValue);
        alpha += 1;
    }

    // potentially this can be off the end of our "legal" alpha values, but that
    // only happens if stopAlpha is also 0. Rather than test for stopAlpha != 0
    // every time (slow), we just do it, and ensure that we've allocated extra space
    // (see the + 1 comment in fStorage[]
    saturated_add(alpha, stopAlpha);
}

void MaskSuperBlitter::blitH(int x, int y, int width) {
    int iy = (y >> SHIFT);

    SkASSERT(iy >= fMask.fBounds.fTop && iy < fMask.fBounds.fBottom);
    iy -= fMask.fBounds.fTop;   // make it relative to 0

    // This should never happen, but it does.  Until the true cause is
    // discovered, let's skip this span instead of crashing.
    // See http://crbug.com/17569.
    if (iy < 0) {
        return;
    }

#ifdef SK_DEBUG
    {
        int ix = x >> SHIFT;
        SkASSERT(ix >= fMask.fBounds.fLeft && ix < fMask.fBounds.fRight);
    }
#endif

    x -= SkLeftShift(fMask.fBounds.fLeft, SHIFT);

    // hack, until I figure out why my cubics (I think) go beyond the bounds
    if (x < 0) {
        width += x;
        x = 0;
    }

    uint8_t* row = fMask.fImage + iy * fMask.fRowBytes + (x >> SHIFT);

    int start = x;
    int stop = x + width;

    SkASSERT(start >= 0 && stop > start);
    int fb = start & MASK;
    int fe = stop & MASK;
    int n = (stop >> SHIFT) - (start >> SHIFT) - 1;


    if (n < 0) {
        SkASSERT(row >= fMask.fImage);
        SkASSERT(row < fMask.fImage + kMAX_STORAGE + 1);
        add_aa_span(row, coverage_to_partial_alpha(fe - fb));
    } else {
        fb = SCALE - fb;
        SkASSERT(row >= fMask.fImage);
        SkASSERT(row + n + 1 < fMask.fImage + kMAX_STORAGE + 1);
        add_aa_span(row,  coverage_to_partial_alpha(fb),
                    n, coverage_to_partial_alpha(fe),
                    (1 << (8 - SHIFT)) - (((y & MASK) + 1) >> SHIFT));
    }

#ifdef SK_DEBUG
    fCurrX = x + width;
#endif
}

///////////////////////////////////////////////////////////////////////////////

void SkScan::AntiFillPath(const SkPath& path, const SkRegion& origClip,
                          SkBlitter* blitter, bool forceRLE) {
    FillPathFunc fillPathFunc = [](const SkPath& path, SkBlitter* blitter, bool isInverse,
            const SkIRect& ir, const SkRegion* clipRgn, const SkIRect* clipRect, bool forceRLE){
        SkIRect superRect, *superClipRect = nullptr;
        if (clipRect) {
            superRect.set(SkLeftShift(clipRect->fLeft, SHIFT),
                          SkLeftShift(clipRect->fTop, SHIFT),
                          SkLeftShift(clipRect->fRight, SHIFT),
                          SkLeftShift(clipRect->fBottom, SHIFT));
            superClipRect = &superRect;
        }

        // MaskSuperBlitter can't handle drawing outside of ir, so we can't use it
        // if we're an inverse filltype
        if (!isInverse && MaskSuperBlitter::CanHandleRect(ir) && !forceRLE) {
            MaskSuperBlitter    superBlit(blitter, ir, *clipRgn, isInverse);
            SkASSERT(SkIntToScalar(ir.fTop) <= path.getBounds().fTop);
            sk_fill_path(path, clipRgn->getBounds(), &superBlit, ir.fTop, ir.fBottom, SHIFT,
                    superClipRect == nullptr);
        } else {
            SuperBlitter    superBlit(blitter, ir, *clipRgn, isInverse);
            sk_fill_path(path, clipRgn->getBounds(), &superBlit, ir.fTop, ir.fBottom, SHIFT,
                    superClipRect == nullptr);
        }
    };

    do_fill_path(path, origClip, blitter, forceRLE, SHIFT, std::move(fillPathFunc));
}

///////////////////////////////////////////////////////////////////////////////

#include "SkRasterClip.h"

void SkScan::FillPath(const SkPath& path, const SkRasterClip& clip,
                          SkBlitter* blitter) {
    if (clip.isEmpty()) {
        return;
    }

    if (clip.isBW()) {
        FillPath(path, clip.bwRgn(), blitter);
    } else {
        SkRegion        tmp;
        SkAAClipBlitter aaBlitter;

        tmp.setRect(clip.getBounds());
        aaBlitter.init(blitter, &clip.aaRgn());
        SkScan::FillPath(path, tmp, &aaBlitter);
    }
}

static bool suitableForAAA(const SkPath& path) {
    if (gSkForceAnalyticAA.load()) {
        return true;
    }
    if (path.isRect(nullptr)) {
        return true;
    }
    const SkRect& bounds = path.getBounds();
    // When the path have so many points compared to the size of its bounds/resolution,
    // it indicates that the path is not quite smooth in the current resolution:
    // the expected number of turning points in every pixel row/column is significantly greater than
    // zero. Hence Aanlytic AA is not likely to produce visible quality improvements, and Analytic
    // AA might be slower than supersampling.
    return path.countPoints() < SkTMax(bounds.width(), bounds.height()) / 2 - 10;
}

void SkScan::AntiFillPath(const SkPath& path, const SkRasterClip& clip,
                          SkBlitter* blitter) {
    if (clip.isEmpty()) {
        return;
    }

    using FillPathProc = void(*)(const SkPath&, const SkRegion&, SkBlitter*, bool);
    FillPathProc fillPathProc = &SkScan::AntiFillPath;

    // Do not use AAA if path is too complicated:
    // there won't be any speedup or significant visual improvement.
    if (gSkUseAnalyticAA.load() && suitableForAAA(path)) {
        fillPathProc = &SkScan::AAAFillPath;
    }

    if (clip.isBW()) {
        fillPathProc(path, clip.bwRgn(), blitter, false);
    } else {
        SkRegion        tmp;
        SkAAClipBlitter aaBlitter;

        tmp.setRect(clip.getBounds());
        aaBlitter.init(blitter, &clip.aaRgn());
        fillPathProc(path, tmp, &aaBlitter, true);
    }
}
