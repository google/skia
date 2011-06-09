/* libs/graphics/sgl/SkScan_AntiPath.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
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

/*
    We have two techniques for capturing the output of the supersampler:
    - SUPERMASK, which records a large mask-bitmap
        this is often faster for small, complex objects
    - RLE, which records a rle-encoded scanline
        this is often faster for large objects with big spans

    NEW_AA is a set of code-changes to try to make both paths produce identical
    results. Its not quite there yet, though the remaining differences may be
    in the subsequent blits, and not in the different masks/runs...
 */
//#define FORCE_SUPERMASK
//#define FORCE_RLE
//#define SK_SUPPORT_NEW_AA

///////////////////////////////////////////////////////////////////////////////

class BaseSuperBlitter : public SkBlitter {
public:
    BaseSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                     const SkRegion& clip);

    virtual void blitAntiH(int x, int y, const SkAlpha antialias[],
                           const int16_t runs[]) {
        SkASSERT(!"How did I get here?");
    }
    virtual void blitV(int x, int y, int height, SkAlpha alpha) {
        SkASSERT(!"How did I get here?");
    }
    virtual void blitRect(int x, int y, int width, int height) {
        SkASSERT(!"How did I get here?");
    }

protected:
    SkBlitter*  fRealBlitter;
    int         fCurrIY;
    int         fWidth, fLeft, fSuperLeft;

    SkDEBUGCODE(int fCurrX;)
    int fCurrY;
};

BaseSuperBlitter::BaseSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                                   const SkRegion& clip) {
    fRealBlitter = realBlitter;

    // take the union of the ir bounds and clip, since we may be called with an
    // inverse filltype
    const int left = SkMin32(ir.fLeft, clip.getBounds().fLeft);
    const int right = SkMax32(ir.fRight, clip.getBounds().fRight);

    fLeft = left;
    fSuperLeft = left << SHIFT;
    fWidth = right - left;
    fCurrIY = -1;
    fCurrY = -1;
    SkDEBUGCODE(fCurrX = -1;)
}

class SuperBlitter : public BaseSuperBlitter {
public:
    SuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                 const SkRegion& clip);

    virtual ~SuperBlitter() {
        this->flush();
        sk_free(fRuns.fRuns);
    }

    void flush();

    virtual void blitH(int x, int y, int width);
    virtual void blitRect(int x, int y, int width, int height);

private:
    SkAlphaRuns fRuns;
    int         fOffsetX;
};

SuperBlitter::SuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                           const SkRegion& clip)
        : BaseSuperBlitter(realBlitter, ir, clip) {
    const int width = fWidth;

    // extra one to store the zero at the end
    fRuns.fRuns = (int16_t*)sk_malloc_throw((width + 1 + (width + 2)/2) * sizeof(int16_t));
    fRuns.fAlpha = (uint8_t*)(fRuns.fRuns + width + 1);
    fRuns.reset(width);

    fOffsetX = 0;
}

void SuperBlitter::flush() {
    if (fCurrIY >= 0) {
        if (!fRuns.empty()) {
        //  SkDEBUGCODE(fRuns.dump();)
            fRealBlitter->blitAntiH(fLeft, fCurrIY, fRuns.fAlpha, fRuns.fRuns);
            fRuns.reset(fWidth);
            fOffsetX = 0;
        }
        fCurrIY = -1;
        SkDEBUGCODE(fCurrX = -1;)
    }
}

static inline int coverage_to_alpha(int aa) {
    aa <<= 8 - 2*SHIFT;
    aa -= aa >> (8 - SHIFT - 1);
    return aa;
}

#define SUPER_Mask      ((1 << SHIFT) - 1)

void SuperBlitter::blitH(int x, int y, int width) {
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

    // we sub 1 from maxValue 1 time for each block, so that we don't
    // hit 256 as a summed max, but 255.
//  int maxValue = (1 << (8 - SHIFT)) - (((y & MASK) + 1) >> SHIFT);

    int start = x;
    int stop = x + width;

    SkASSERT(start >= 0 && stop > start);
    int fb = start & SUPER_Mask;
    int fe = stop & SUPER_Mask;
    int n = (stop >> SHIFT) - (start >> SHIFT) - 1;

    if (n < 0) {
        fb = fe - fb;
        n = 0;
        fe = 0;
    } else {
        if (fb == 0) {
            n += 1;
        } else {
            fb = (1 << SHIFT) - fb;
        }
    }

    fOffsetX = fRuns.add(x >> SHIFT, coverage_to_alpha(fb), n, coverage_to_alpha(fe),
                         (1 << (8 - SHIFT)) - (((y & MASK) + 1) >> SHIFT),
                         fOffsetX);

#ifdef SK_DEBUG
    fRuns.assertValid(y & MASK, (1 << (8 - SHIFT)));
    fCurrX = x + width;
#endif
}

void SuperBlitter::blitRect(int x, int y, int width, int height) {
    for (int i = 0; i < height; ++i) {
        blitH(x, y + i, width);
    }

    flush();
}

///////////////////////////////////////////////////////////////////////////////

class MaskSuperBlitter : public BaseSuperBlitter {
public:
    MaskSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                     const SkRegion& clip);
    virtual ~MaskSuperBlitter() {
        fRealBlitter->blitMask(fMask, fClipRect);
    }

    virtual void blitH(int x, int y, int width);

    static bool CanHandleRect(const SkIRect& bounds) {
#ifdef FORCE_RLE
        return false;
#endif
        int width = bounds.width();
        int rb = SkAlign4(width);

        return (width <= MaskSuperBlitter::kMAX_WIDTH) &&
        (rb * bounds.height() <= MaskSuperBlitter::kMAX_STORAGE);
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

MaskSuperBlitter::MaskSuperBlitter(SkBlitter* realBlitter, const SkIRect& ir,
                                   const SkRegion& clip)
        : BaseSuperBlitter(realBlitter, ir, clip) {
    SkASSERT(CanHandleRect(ir));

    fMask.fImage    = (uint8_t*)fStorage;
    fMask.fBounds   = ir;
    fMask.fRowBytes = ir.width();
    fMask.fFormat   = SkMask::kA8_Format;

    fClipRect = ir;
    fClipRect.intersect(clip.getBounds());

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

// minimum count before we want to setup an inner loop, adding 4-at-a-time
#define MIN_COUNT_FOR_QUAD_LOOP  16

static void add_aa_span(uint8_t* alpha, U8CPU startAlpha, int middleCount,
                        U8CPU stopAlpha, U8CPU maxValue) {
    SkASSERT(middleCount >= 0);

    /*  I should be able to just add alpha[x] + startAlpha.
        However, if the trailing edge of the previous span and the leading
        edge of the current span round to the same super-sampled x value,
        I might overflow to 256 with this add, hence the funny subtract.
    */
#ifdef SK_SUPPORT_NEW_AA
    if (startAlpha) {
        unsigned tmp = *alpha + startAlpha;
        SkASSERT(tmp <= 256);
        *alpha++ = SkToU8(tmp - (tmp >> 8));
    }
#else
    unsigned tmp = *alpha + startAlpha;
    SkASSERT(tmp <= 256);
    *alpha++ = SkToU8(tmp - (tmp >> 8));
#endif

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
    *alpha = SkToU8(*alpha + stopAlpha);
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

    x -= (fMask.fBounds.fLeft << SHIFT);

    // hack, until I figure out why my cubics (I think) go beyond the bounds
    if (x < 0) {
        width += x;
        x = 0;
    }

    // we sub 1 from maxValue 1 time for each block, so that we don't
    // hit 256 as a summed max, but 255.
//  int maxValue = (1 << (8 - SHIFT)) - (((y & MASK) + 1) >> SHIFT);

    uint8_t* row = fMask.fImage + iy * fMask.fRowBytes + (x >> SHIFT);

    int start = x;
    int stop = x + width;

    SkASSERT(start >= 0 && stop > start);
    int fb = start & SUPER_Mask;
    int fe = stop & SUPER_Mask;
    int n = (stop >> SHIFT) - (start >> SHIFT) - 1;


    if (n < 0) {
        SkASSERT(row >= fMask.fImage);
        SkASSERT(row < fMask.fImage + kMAX_STORAGE + 1);
        add_aa_span(row, coverage_to_alpha(fe - fb));
    } else {
#ifdef SK_SUPPORT_NEW_AA
        if (0 == fb) {
            n += 1;
        } else {
            fb = (1 << SHIFT) - fb;
        }
#else
        fb = (1 << SHIFT) - fb;
#endif
        SkASSERT(row >= fMask.fImage);
        SkASSERT(row + n + 1 < fMask.fImage + kMAX_STORAGE + 1);
        add_aa_span(row,  coverage_to_alpha(fb), n, coverage_to_alpha(fe),
                    (1 << (8 - SHIFT)) - (((y & MASK) + 1) >> SHIFT));
    }

#ifdef SK_DEBUG
    fCurrX = x + width;
#endif
}

///////////////////////////////////////////////////////////////////////////////

/*  Returns non-zero if (value << shift) overflows a short, which would mean
    we could not shift it up and then convert to SkFixed.
    i.e. is x expressible as signed (16-shift) bits?
 */
static int overflows_short_shift(int value, int shift) {
    const int s = 16 + shift;
    return (value << s >> s) - value;
}

void SkScan::AntiFillPath(const SkPath& path, const SkRegion& clip,
                          SkBlitter* blitter) {
    if (clip.isEmpty()) {
        return;
    }

    SkIRect ir;
    path.getBounds().roundOut(&ir);
    if (ir.isEmpty()) {
        return;
    }

    // use bit-or since we expect all to pass, so no need to go slower with
    // a short-circuiting logical-or
    if (overflows_short_shift(ir.fLeft, SHIFT) |
            overflows_short_shift(ir.fRight, SHIFT) |
            overflows_short_shift(ir.fTop, SHIFT) |
            overflows_short_shift(ir.fBottom, SHIFT)) {
        // can't supersample, so draw w/o antialiasing
        SkScan::FillPath(path, clip, blitter);
        return;
    }

    SkScanClipper   clipper(blitter, &clip, ir);
    const SkIRect*  clipRect = clipper.getClipRect();

    if (clipper.getBlitter() == NULL) { // clipped out
        if (path.isInverseFillType()) {
            blitter->blitRegion(clip);
        }
        return;
    }

    // now use the (possibly wrapped) blitter
    blitter = clipper.getBlitter();

    if (path.isInverseFillType()) {
        sk_blit_above(blitter, ir, clip);
    }

    SkIRect superRect, *superClipRect = NULL;

    if (clipRect) {
        superRect.set(  clipRect->fLeft << SHIFT, clipRect->fTop << SHIFT,
                        clipRect->fRight << SHIFT, clipRect->fBottom << SHIFT);
        superClipRect = &superRect;
    }

    SkASSERT(SkIntToScalar(ir.fTop) <= path.getBounds().fTop);

    // MaskSuperBlitter can't handle drawing outside of ir, so we can't use it
    // if we're an inverse filltype
    if (!path.isInverseFillType() && MaskSuperBlitter::CanHandleRect(ir)) {
        MaskSuperBlitter    superBlit(blitter, ir, clip);
        SkASSERT(SkIntToScalar(ir.fTop) <= path.getBounds().fTop);
        sk_fill_path(path, superClipRect, &superBlit, ir.fTop, ir.fBottom, SHIFT, clip);
    } else {
        SuperBlitter    superBlit(blitter, ir, clip);
        sk_fill_path(path, superClipRect, &superBlit, ir.fTop, ir.fBottom, SHIFT, clip);
    }

    if (path.isInverseFillType()) {
        sk_blit_below(blitter, ir, clip);
    }
}
