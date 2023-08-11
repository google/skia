/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlitter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkAlphaRuns.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMemset.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkRegionPriv.h"
#include "src/shaders/SkShaderBase.h"

#include <cstddef>
#include <functional>
#include <optional>

using namespace skia_private;

// Hacks for testing.
bool gSkForceRasterPipelineBlitter{false};

SkBlitter::~SkBlitter() {}

bool SkBlitter::isNullBlitter() const { return false; }

/*
void SkBlitter::blitH(int x, int y, int width) {
    SkDEBUGFAIL("unimplemented");
}


void SkBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                          const int16_t runs[]) {
    SkDEBUGFAIL("unimplemented");
}
 */

inline static SkAlpha ScalarToAlpha(SkScalar a) {
    SkAlpha alpha = (SkAlpha)(a * 255);
    return alpha > 247 ? 0xFF : alpha < 8 ? 0 : alpha;
}

void SkBlitter::blitFatAntiRect(const SkRect& rect) {
    SkIRect bounds = rect.roundOut();
    SkASSERT(bounds.width() >= 3);

    // skbug.com/7813
    // To ensure consistency of the threaded backend (a rect that's considered fat in the init-once
    // phase must also be considered fat in the draw phase), we have to deal with rects with small
    // heights because the horizontal tiling in the threaded backend may change the height.
    //
    // This also implies that we cannot do vertical tiling unless we can blit any rect (not just the
    // fat one.)
    if (bounds.height() == 0) {
        return;
    }

    int         runSize = bounds.width() + 1; // +1 so we can set runs[bounds.width()] = 0
    void*       storage = this->allocBlitMemory(runSize * (sizeof(int16_t) + sizeof(SkAlpha)));
    int16_t*    runs    = reinterpret_cast<int16_t*>(storage);
    SkAlpha*    alphas  = reinterpret_cast<SkAlpha*>(runs + runSize);

    runs[0] = 1;
    runs[1] = bounds.width() - 2;
    runs[bounds.width() - 1] = 1;
    runs[bounds.width()]  = 0;

    SkScalar partialL = bounds.fLeft + 1 - rect.fLeft;
    SkScalar partialR = rect.fRight - (bounds.fRight - 1);
    SkScalar partialT = bounds.fTop + 1 - rect.fTop;
    SkScalar partialB = rect.fBottom - (bounds.fBottom - 1);

    if (bounds.height() == 1) {
        partialT = rect.fBottom - rect.fTop;
    }

    alphas[0] = ScalarToAlpha(partialL * partialT);
    alphas[1] = ScalarToAlpha(partialT);
    alphas[bounds.width() - 1] = ScalarToAlpha(partialR * partialT);
    this->blitAntiH(bounds.fLeft, bounds.fTop, alphas, runs);

    if (bounds.height() > 2) {
        this->blitAntiRect(bounds.fLeft, bounds.fTop + 1, bounds.width() - 2, bounds.height() - 2,
                           ScalarToAlpha(partialL), ScalarToAlpha(partialR));
    }

    if (bounds.height() > 1) {
        alphas[0] = ScalarToAlpha(partialL * partialB);
        alphas[1] = ScalarToAlpha(partialB);
        alphas[bounds.width() - 1] = ScalarToAlpha(partialR * partialB);
        this->blitAntiH(bounds.fLeft, bounds.fBottom - 1, alphas, runs);
    }
}

void SkBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 255) {
        this->blitRect(x, y, 1, height);
    } else {
        int16_t runs[2];
        runs[0] = 1;
        runs[1] = 0;

        while (--height >= 0) {
            this->blitAntiH(x, y++, &alpha, runs);
        }
    }
}

void SkBlitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(width > 0);
    while (--height >= 0) {
        this->blitH(x, y++, width);
    }
}

/// Default implementation doesn't check for easy optimizations
/// such as alpha == 255; also uses blitV(), which some subclasses
/// may not support.
void SkBlitter::blitAntiRect(int x, int y, int width, int height,
                             SkAlpha leftAlpha, SkAlpha rightAlpha) {
    if (leftAlpha > 0) { // we may send in x = -1 with leftAlpha = 0
        this->blitV(x, y, height, leftAlpha);
    }
    x++;
    if (width > 0) {
        this->blitRect(x, y, width, height);
        x += width;
    }
    if (rightAlpha > 0) {
        this->blitV(x, y, height, rightAlpha);
    }
}

//////////////////////////////////////////////////////////////////////////////

static inline void bits_to_runs(SkBlitter* blitter, int x, int y,
                                const uint8_t bits[],
                                uint8_t left_mask, ptrdiff_t rowBytes,
                                uint8_t right_mask) {
    int inFill = 0;
    int pos = 0;

    while (--rowBytes >= 0) {
        uint8_t b = *bits++ & left_mask;
        if (rowBytes == 0) {
            b &= right_mask;
        }

        for (uint8_t test = 0x80U; test != 0; test >>= 1) {
            if (b & test) {
                if (!inFill) {
                    pos = x;
                    inFill = true;
                }
            } else {
                if (inFill) {
                    blitter->blitH(pos, y, x - pos);
                    inFill = false;
                }
            }
            x += 1;
        }
        left_mask = 0xFFU;
    }

    // final cleanup
    if (inFill) {
        blitter->blitH(pos, y, x - pos);
    }
}

// maskBitCount is the number of 1's to place in the mask. It must be in the range between 1 and 8.
static uint8_t generate_right_mask(int maskBitCount) {
    return static_cast<uint8_t>((0xFF00U >> maskBitCount) & 0xFF);
}

void SkBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (mask.fFormat == SkMask::kLCD16_Format) {
        return; // needs to be handled by subclass
    }

    if (mask.fFormat == SkMask::kBW_Format) {
        int cx = clip.fLeft;
        int cy = clip.fTop;
        int maskLeft = mask.fBounds.fLeft;
        int maskRowBytes = mask.fRowBytes;
        int height = clip.height();

        const uint8_t* bits = mask.getAddr1(cx, cy);

        SkDEBUGCODE(const uint8_t* endOfImage =
            mask.fImage + (mask.fBounds.height() - 1) * maskRowBytes
            + ((mask.fBounds.width() + 7) >> 3));

        if (cx == maskLeft && clip.fRight == mask.fBounds.fRight) {
            while (--height >= 0) {
                int affectedRightBit = mask.fBounds.width() - 1;
                ptrdiff_t rowBytes = (affectedRightBit >> 3) + 1;
                SkASSERT(bits + rowBytes <= endOfImage);
                U8CPU rightMask = generate_right_mask((affectedRightBit & 7) + 1);
                bits_to_runs(this, cx, cy, bits, 0xFF, rowBytes, rightMask);
                bits += maskRowBytes;
                cy += 1;
            }
        } else {
            // Bits is calculated as the offset into the mask at the point {cx, cy} therefore, all
            // addressing into the bit mask is relative to that point. Since this is an address
            // calculated from a arbitrary bit in that byte, calculate the left most bit.
            int bitsLeft = cx - ((cx - maskLeft) & 7);

            // Everything is relative to the bitsLeft.
            int leftEdge = cx - bitsLeft;
            SkASSERT(leftEdge >= 0);
            int rightEdge = clip.fRight - bitsLeft;
            SkASSERT(rightEdge > leftEdge);

            // Calculate left byte and mask
            const uint8_t* leftByte = bits;
            U8CPU leftMask = 0xFFU >> (leftEdge & 7);

            // Calculate right byte and mask
            int affectedRightBit = rightEdge - 1;
            const uint8_t* rightByte = bits + (affectedRightBit >> 3);
            U8CPU rightMask = generate_right_mask((affectedRightBit & 7) + 1);

            // leftByte and rightByte are byte locations therefore, to get a count of bytes the
            // code must add one.
            ptrdiff_t rowBytes = rightByte - leftByte + 1;

            while (--height >= 0) {
                SkASSERT(bits + rowBytes <= endOfImage);
                bits_to_runs(this, bitsLeft, cy, bits, leftMask, rowBytes, rightMask);
                bits += maskRowBytes;
                cy += 1;
            }
        }
    } else {
        int                         width = clip.width();
        AutoSTMalloc<64, int16_t> runStorage(width + 1);
        int16_t*                    runs = runStorage.get();
        const uint8_t*              aa = mask.getAddr8(clip.fLeft, clip.fTop);

        SkOpts::memset16((uint16_t*)runs, 1, width);
        runs[width] = 0;

        int height = clip.height();
        int y = clip.fTop;
        while (--height >= 0) {
            this->blitAntiH(clip.fLeft, y, aa, runs);
            aa += mask.fRowBytes;
            y += 1;
        }
    }
}

/////////////////////// these are not virtual, just helpers

#if defined(SK_SUPPORT_LEGACY_ALPHA_BITMAP_AS_COVERAGE)
void SkBlitter::blitMaskRegion(const SkMask& mask, const SkRegion& clip) {
    if (clip.quickReject(mask.fBounds)) {
        return;
    }

    SkRegion::Cliperator clipper(clip, mask.fBounds);

    while (!clipper.done()) {
        const SkIRect& cr = clipper.rect();
        this->blitMask(mask, cr);
        clipper.next();
    }
}
#endif

void SkBlitter::blitRectRegion(const SkIRect& rect, const SkRegion& clip) {
    SkRegion::Cliperator clipper(clip, rect);

    while (!clipper.done()) {
        const SkIRect& cr = clipper.rect();
        this->blitRect(cr.fLeft, cr.fTop, cr.width(), cr.height());
        clipper.next();
    }
}

void SkBlitter::blitRegion(const SkRegion& clip) {
    SkRegionPriv::VisitSpans(clip, [this](const SkIRect& r) {
        this->blitRect(r.left(), r.top(), r.width(), r.height());
    });
}

///////////////////////////////////////////////////////////////////////////////

void SkNullBlitter::blitH(int x, int y, int width) {}

void SkNullBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                              const int16_t runs[]) {}

void SkNullBlitter::blitV(int x, int y, int height, SkAlpha alpha) {}

void SkNullBlitter::blitRect(int x, int y, int width, int height) {}

void SkNullBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {}

bool SkNullBlitter::isNullBlitter() const { return true; }

///////////////////////////////////////////////////////////////////////////////

static int compute_anti_width(const int16_t runs[]) {
    int width = 0;

    for (;;) {
        int count = runs[0];

        SkASSERT(count >= 0);
        if (count == 0) {
            break;
        }
        width += count;
        runs += count;
    }
    return width;
}

static inline bool y_in_rect(int y, const SkIRect& rect) {
    return (unsigned)(y - rect.fTop) < (unsigned)rect.height();
}

static inline bool x_in_rect(int x, const SkIRect& rect) {
    return (unsigned)(x - rect.fLeft) < (unsigned)rect.width();
}

void SkRectClipBlitter::blitH(int left, int y, int width) {
    SkASSERT(width > 0);

    if (!y_in_rect(y, fClipRect)) {
        return;
    }

    int right = left + width;

    if (left < fClipRect.fLeft) {
        left = fClipRect.fLeft;
    }
    if (right > fClipRect.fRight) {
        right = fClipRect.fRight;
    }

    width = right - left;
    if (width > 0) {
        fBlitter->blitH(left, y, width);
    }
}

void SkRectClipBlitter::blitAntiH(int left, int y, const SkAlpha aa[],
                                  const int16_t runs[]) {
    if (!y_in_rect(y, fClipRect) || left >= fClipRect.fRight) {
        return;
    }

    int x0 = left;
    int x1 = left + compute_anti_width(runs);

    if (x1 <= fClipRect.fLeft) {
        return;
    }

    SkASSERT(x0 < x1);
    if (x0 < fClipRect.fLeft) {
        int dx = fClipRect.fLeft - x0;
        SkAlphaRuns::BreakAt((int16_t*)runs, (uint8_t*)aa, dx);
        runs += dx;
        aa += dx;
        x0 = fClipRect.fLeft;
    }

    SkASSERT(x0 < x1 && runs[x1 - x0] == 0);
    if (x1 > fClipRect.fRight) {
        x1 = fClipRect.fRight;
        SkAlphaRuns::BreakAt((int16_t*)runs, (uint8_t*)aa, x1 - x0);
        ((int16_t*)runs)[x1 - x0] = 0;
    }

    SkASSERT(x0 < x1 && runs[x1 - x0] == 0);
    SkASSERT(compute_anti_width(runs) == x1 - x0);

    fBlitter->blitAntiH(x0, y, aa, runs);
}

void SkRectClipBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(height > 0);

    if (!x_in_rect(x, fClipRect)) {
        return;
    }

    int y0 = y;
    int y1 = y + height;

    if (y0 < fClipRect.fTop) {
        y0 = fClipRect.fTop;
    }
    if (y1 > fClipRect.fBottom) {
        y1 = fClipRect.fBottom;
    }

    if (y0 < y1) {
        fBlitter->blitV(x, y0, y1 - y0, alpha);
    }
}

void SkRectClipBlitter::blitRect(int left, int y, int width, int height) {
    SkIRect    r;

    r.setLTRB(left, y, left + width, y + height);
    if (r.intersect(fClipRect)) {
        fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
    }
}

void SkRectClipBlitter::blitAntiRect(int left, int y, int width, int height,
                                     SkAlpha leftAlpha, SkAlpha rightAlpha) {
    SkIRect    r;

    // The *true* width of the rectangle blitted is width+2:
    r.setLTRB(left, y, left + width + 2, y + height);
    if (r.intersect(fClipRect)) {
        if (r.fLeft != left) {
            SkASSERT(r.fLeft > left);
            leftAlpha = 255;
        }
        if (r.fRight != left + width + 2) {
            SkASSERT(r.fRight < left + width + 2);
            rightAlpha = 255;
        }
        if (255 == leftAlpha && 255 == rightAlpha) {
            fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
        } else if (1 == r.width()) {
            if (r.fLeft == left) {
                fBlitter->blitV(r.fLeft, r.fTop, r.height(), leftAlpha);
            } else {
                SkASSERT(r.fLeft == left + width + 1);
                fBlitter->blitV(r.fLeft, r.fTop, r.height(), rightAlpha);
            }
        } else {
            fBlitter->blitAntiRect(r.fLeft, r.fTop, r.width() - 2, r.height(),
                                   leftAlpha, rightAlpha);
        }
    }
}

void SkRectClipBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    SkIRect    r = clip;

    if (r.intersect(fClipRect)) {
        fBlitter->blitMask(mask, r);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkRgnClipBlitter::blitH(int x, int y, int width) {
    SkRegion::Spanerator span(*fRgn, y, x, x + width);
    int left, right;

    while (span.next(&left, &right)) {
        SkASSERT(left < right);
        fBlitter->blitH(left, y, right - left);
    }
}

void SkRgnClipBlitter::blitAntiH(int x, int y, const SkAlpha aa[],
                                 const int16_t runs[]) {
    int width = compute_anti_width(runs);
    SkRegion::Spanerator span(*fRgn, y, x, x + width);
    int left, right;
    SkDEBUGCODE(const SkIRect& bounds = fRgn->getBounds();)

    int prevRite = x;
    while (span.next(&left, &right)) {
        SkASSERT(x <= left);
        SkASSERT(left < right);
        SkASSERT(left >= bounds.fLeft && right <= bounds.fRight);

        SkAlphaRuns::Break((int16_t*)runs, (uint8_t*)aa, left - x, right - left);

        // now zero before left
        if (left > prevRite) {
            int index = prevRite - x;
            ((uint8_t*)aa)[index] = 0;   // skip runs after right
            ((int16_t*)runs)[index] = SkToS16(left - prevRite);
        }

        prevRite = right;
    }

    if (prevRite > x) {
        ((int16_t*)runs)[prevRite - x] = 0;

        if (x < 0) {
            int skip = runs[0];
            SkASSERT(skip >= -x);
            aa += skip;
            runs += skip;
            x += skip;
        }
        fBlitter->blitAntiH(x, y, aa, runs);
    }
}

void SkRgnClipBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkIRect    bounds;
    bounds.setXYWH(x, y, 1, height);

    SkRegion::Cliperator    iter(*fRgn, bounds);

    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));

        fBlitter->blitV(x, r.fTop, r.height(), alpha);
        iter.next();
    }
}

void SkRgnClipBlitter::blitRect(int x, int y, int width, int height) {
    SkIRect    bounds;
    bounds.setXYWH(x, y, width, height);

    SkRegion::Cliperator    iter(*fRgn, bounds);

    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));

        fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
        iter.next();
    }
}

void SkRgnClipBlitter::blitAntiRect(int x, int y, int width, int height,
                                    SkAlpha leftAlpha, SkAlpha rightAlpha) {
    // The *true* width of the rectangle to blit is width + 2
    SkIRect    bounds;
    bounds.setXYWH(x, y, width + 2, height);

    SkRegion::Cliperator    iter(*fRgn, bounds);

    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));
        SkASSERT(r.fLeft >= x);
        SkASSERT(r.fRight <= x + width + 2);

        SkAlpha effectiveLeftAlpha = (r.fLeft == x) ? leftAlpha : 255;
        SkAlpha effectiveRightAlpha = (r.fRight == x + width + 2) ?
                                      rightAlpha : 255;

        if (255 == effectiveLeftAlpha && 255 == effectiveRightAlpha) {
            fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
        } else if (1 == r.width()) {
            if (r.fLeft == x) {
                fBlitter->blitV(r.fLeft, r.fTop, r.height(),
                                effectiveLeftAlpha);
            } else {
                SkASSERT(r.fLeft == x + width + 1);
                fBlitter->blitV(r.fLeft, r.fTop, r.height(),
                                effectiveRightAlpha);
            }
        } else {
            fBlitter->blitAntiRect(r.fLeft, r.fTop, r.width() - 2, r.height(),
                                   effectiveLeftAlpha, effectiveRightAlpha);
        }
        iter.next();
    }
}


void SkRgnClipBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    SkRegion::Cliperator iter(*fRgn, clip);
    const SkIRect&       r = iter.rect();
    SkBlitter*           blitter = fBlitter;

    while (!iter.done()) {
        blitter->blitMask(mask, r);
        iter.next();
    }
}

///////////////////////////////////////////////////////////////////////////////

SkBlitter* SkBlitterClipper::apply(SkBlitter* blitter, const SkRegion* clip,
                                   const SkIRect* ir) {
    if (clip) {
        const SkIRect& clipR = clip->getBounds();

        if (clip->isEmpty() || (ir && !SkIRect::Intersects(clipR, *ir))) {
            blitter = &fNullBlitter;
        } else if (clip->isRect()) {
            if (ir == nullptr || !clipR.contains(*ir)) {
                fRectBlitter.init(blitter, clipR);
                blitter = &fRectBlitter;
            }
        } else {
            fRgnBlitter.init(blitter, clip);
            blitter = &fRgnBlitter;
        }
    }
    return blitter;
}

///////////////////////////////////////////////////////////////////////////////

bool SkBlitter::UseLegacyBlitter(const SkPixmap& device,
                                 const SkPaint& paint,
                                 const SkMatrix& matrix) {
    if (gSkForceRasterPipelineBlitter) {
        return false;
    }
#if defined(SK_FORCE_RASTER_PIPELINE_BLITTER)
    return false;
#else

    if (paint.isDither()) {
        return false;
    }

    const SkMaskFilterBase* mf = as_MFB(paint.getMaskFilter());

    // The legacy blitters cannot handle any of these "complex" features (anymore).
    if (device.alphaType() == kUnpremul_SkAlphaType   ||
        !paint.isSrcOver()                            ||
        (mf && mf->getFormat() == SkMask::k3D_Format)) {
        return false;
    }

    auto cs = device.colorSpace();
    // We check (indirectly via makeContext()) later on if the shader can handle the colorspace
    // in legacy mode, so here we just focus on if a single color needs raster-pipeline.
    if (cs && !paint.getShader()) {
        if (!paint.getColor4f().fitsInBytes() || !cs->isSRGB()) {
            return false;
        }
    }

    // Only kN32 is handled by legacy blitters now
    return device.colorType() == kN32_SkColorType;
#endif
}

SkBlitter* SkBlitter::Choose(const SkPixmap& device,
                             const SkMatrix& ctm,
                             const SkPaint& origPaint,
                             SkArenaAlloc* alloc,
                             bool drawCoverage,
                             sk_sp<SkShader> clipShader,
                             const SkSurfaceProps& props) {
    SkASSERT(alloc);

    if (kUnknown_SkColorType == device.colorType()) {
        return alloc->make<SkNullBlitter>();
    }

    // We may tweak the original paint as we go.
    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    if (auto mode = paint->asBlendMode()) {
        // We have the most fast-paths for SrcOver, so see if we can act like SrcOver.
        if (mode.value() != SkBlendMode::kSrcOver) {
            switch (CheckFastPath(*paint, SkColorTypeIsAlwaysOpaque(device.colorType()))) {
                case SkBlendFastPath::kSrcOver:
                    paint.writable()->setBlendMode(SkBlendMode::kSrcOver);
                    break;
                case SkBlendFastPath::kSkipDrawing:
                    return alloc->make<SkNullBlitter>();
                default:
                    break;
            }
        }

        // A Clear blend mode will ignore the entire color pipeline, as if Src mode with 0x00000000.
        if (mode.value() == SkBlendMode::kClear) {
            SkPaint* p = paint.writable();
            p->setShader(nullptr);
            p->setColorFilter(nullptr);
            p->setBlendMode(SkBlendMode::kSrc);
            p->setColor(0x00000000);
        }
    }

    if (paint->getColorFilter()) {
        SkPaintPriv::RemoveColorFilter(paint.writable(), device.colorSpace());
    }
    SkASSERT(!paint->getColorFilter());

    if (drawCoverage) {
        if (device.colorType() == kAlpha_8_SkColorType) {
            SkASSERT(!paint->getShader());
            SkASSERT(paint->isSrcOver());
            return alloc->make<SkA8_Coverage_Blitter>(device, *paint);
        }
        return alloc->make<SkNullBlitter>();
    }

    if (paint->isDither() && !SkPaintPriv::ShouldDither(*paint, device.colorType())) {
        paint.writable()->setDither(false);
    }

    auto CreateSkRPBlitter = [&]() -> SkBlitter* {
        auto blitter = SkCreateRasterPipelineBlitter(device, *paint, ctm, alloc, clipShader, props);
        return blitter ? blitter
                       : alloc->make<SkNullBlitter>();
    };

    // We'll end here for many interesting cases: color spaces, color filters, most color types.
    if (clipShader || !UseLegacyBlitter(device, *paint, ctm)) {
        return CreateSkRPBlitter();
    }

    // Everything but legacy kN32_SkColorType should already be handled.
    SkASSERT(device.colorType() == kN32_SkColorType);

    // And we should be blending with SrcOver
    SkASSERT(paint->asBlendMode() == SkBlendMode::kSrcOver);

    // Legacy blitters keep their shader state on a shader context.
    SkShaderBase::Context* shaderContext = nullptr;
    if (paint->getShader()) {
        shaderContext = as_SB(paint->getShader())
                                ->makeContext({paint->getAlpha(),
                                               SkShaders::MatrixRec(ctm),
                                               device.colorType(),
                                               device.colorSpace(),
                                               props},
                                              alloc);

        // Creating the context isn't always possible... try fallbacks before giving up.
        if (!shaderContext) {
            return CreateSkRPBlitter();
        }
    }

    if (shaderContext) {
        return alloc->make<SkARGB32_Shader_Blitter>(device, *paint, shaderContext);
    } else if (paint->getColor() == SK_ColorBLACK) {
        return alloc->make<SkARGB32_Black_Blitter>(device, *paint);
    } else if (paint->getAlpha() == 0xFF) {
        return alloc->make<SkARGB32_Opaque_Blitter>(device, *paint);
    } else {
        return alloc->make<SkARGB32_Blitter>(device, *paint);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkShaderBlitter::SkShaderBlitter(const SkPixmap& device, const SkPaint& paint,
                                 SkShaderBase::Context* shaderContext)
        : INHERITED(device)
        , fShader(paint.refShader())
        , fShaderContext(shaderContext) {
    SkASSERT(fShader);
    SkASSERT(fShaderContext);
}

SkShaderBlitter::~SkShaderBlitter() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkRectClipCheckBlitter::blitH(int x, int y, int width) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, width, 1)));
    fBlitter->blitH(x, y, width);
}

void SkRectClipCheckBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    const int16_t* iter = runs;
    for (; *iter; iter += *iter)
        ;
    int width = iter - runs;
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, width, 1)));
    fBlitter->blitAntiH(x, y, aa, runs);
}

void SkRectClipCheckBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, 1, height)));
    fBlitter->blitV(x, y, height, alpha);
}

void SkRectClipCheckBlitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, width, height)));
    fBlitter->blitRect(x, y, width, height);
}

void SkRectClipCheckBlitter::blitAntiRect(int x, int y, int width, int height,
                                     SkAlpha leftAlpha, SkAlpha rightAlpha) {
    bool skipLeft = !leftAlpha;
    bool skipRight = !rightAlpha;
    SkIRect r = SkIRect::MakeXYWH(x + skipLeft, y, width + 2 - skipRight - skipLeft, height);
    SkASSERT(r.isEmpty() || fClipRect.contains(r));
    fBlitter->blitAntiRect(x, y, width, height, leftAlpha, rightAlpha);
}

void SkRectClipCheckBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));
    SkASSERT(fClipRect.contains(clip));
    fBlitter->blitMask(mask, clip);
}

void SkRectClipCheckBlitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, 2, 1)));
    fBlitter->blitAntiH2(x, y, a0, a1);
}

void SkRectClipCheckBlitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, 1, 2)));
    fBlitter->blitAntiV2(x, y, a0, a1);
}

#endif
