/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDraw.h"

#include "SkGlyphCache.h"
#include "SkPaintPriv.h"
#include "SkRasterClip.h"
#include "SkScalerContext.h"
#include "SkTextToPathIter.h"
#include "SkUtils.h"

bool SkDraw::ShouldDrawTextAsPaths(const SkPaint& paint, const SkMatrix& ctm, SkScalar sizeLimit) {
    // hairline glyphs are fast enough so we don't need to cache them
    if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
        return true;
    }

    // we don't cache perspective
    if (ctm.hasPerspective()) {
        return true;
    }

    SkMatrix textM;
    SkPaintPriv::MakeTextMatrix(&textM, paint);
    return SkPaint::TooBigToUseCache(ctm, textM, sizeLimit);
}

// disable warning : local variable used without having been initialized
#if defined _WIN32
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

class DrawOneGlyph {
public:
    DrawOneGlyph(const SkDraw& draw, const SkPaint& paint, SkGlyphCache* cache, SkBlitter* blitter)
        : fUseRegionToDraw(UsingRegionToDraw(draw.fRC))
        , fGlyphCache(cache)
        , fBlitter(blitter)
        , fClip(fUseRegionToDraw ? &draw.fRC->bwRgn() : nullptr)
        , fDraw(draw)
        , fPaint(paint)
        , fClipBounds(PickClipBounds(draw)) { }

    void operator()(const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
        position += rounding;
        // Prevent glyphs from being drawn outside of or straddling the edge of device space.
        // Comparisons written a little weirdly so that NaN coordinates are treated safely.
        auto gt = [](float a, int b) { return !(a <= (float)b); };
        auto lt = [](float a, int b) { return !(a >= (float)b); };
        if (gt(position.fX, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
            lt(position.fX, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)) ||
            gt(position.fY, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
            lt(position.fY, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/))) {
            return;
        }

        int left = SkScalarFloorToInt(position.fX);
        int top  = SkScalarFloorToInt(position.fY);
        SkASSERT(glyph.fWidth > 0 && glyph.fHeight > 0);

        left += glyph.fLeft;
        top  += glyph.fTop;

        int right   = left + glyph.fWidth;
        int bottom  = top  + glyph.fHeight;

        SkMask mask;
        mask.fBounds.set(left, top, right, bottom);
        SkASSERT(!mask.fBounds.isEmpty());

        if (fUseRegionToDraw) {
            SkRegion::Cliperator clipper(*fClip, mask.fBounds);

            if (!clipper.done() && this->getImageData(glyph, &mask)) {
                if (SkMask::kARGB32_Format == mask.fFormat) {
                    this->blitARGB32Mask(mask);
                } else {
                    const SkIRect& cr = clipper.rect();
                    do {
                        fBlitter->blitMask(mask, cr);
                        clipper.next();
                    } while (!clipper.done());
                }
            }
        } else {
            SkIRect  storage;
            SkIRect* bounds = &mask.fBounds;

            // this extra test is worth it, assuming that most of the time it succeeds
            // since we can avoid writing to storage
            if (!fClipBounds.containsNoEmptyCheck(mask.fBounds)) {
                if (!storage.intersectNoEmptyCheck(mask.fBounds, fClipBounds)) {
                    return;
                }
                bounds = &storage;
            }

            if (this->getImageData(glyph, &mask)) {
                if (SkMask::kARGB32_Format == mask.fFormat) {
                    this->blitARGB32Mask(mask);
                } else {
                    fBlitter->blitMask(mask, *bounds);
                }
            }
        }
    }

private:
    static bool UsingRegionToDraw(const SkRasterClip* rClip) {
        return rClip->isBW() && !rClip->isRect();
    }

    static SkIRect PickClipBounds(const SkDraw& draw) {
        const SkRasterClip& rasterClip = *draw.fRC;

        if (rasterClip.isBW()) {
            return rasterClip.bwRgn().getBounds();
        } else {
            return rasterClip.aaRgn().getBounds();
        }
    }

    bool getImageData(const SkGlyph& glyph, SkMask* mask) {
        uint8_t* bits = (uint8_t*)(fGlyphCache->findImage(glyph));
        if (nullptr == bits) {
            return false;  // can't rasterize glyph
        }
        mask->fImage    = bits;
        mask->fRowBytes = glyph.rowBytes();
        mask->fFormat   = static_cast<SkMask::Format>(glyph.fMaskFormat);
        return true;
    }

    void blitARGB32Mask(const SkMask& mask) const {
        SkASSERT(SkMask::kARGB32_Format == mask.fFormat);
        SkBitmap bm;
        bm.installPixels(
            SkImageInfo::MakeN32Premul(mask.fBounds.width(), mask.fBounds.height()),
            (SkPMColor*)mask.fImage, mask.fRowBytes);

        fDraw.drawSprite(bm, mask.fBounds.x(), mask.fBounds.y(), fPaint);
    }

    const bool            fUseRegionToDraw;
    SkGlyphCache  * const fGlyphCache;
    SkBlitter     * const fBlitter;
    const SkRegion* const fClip;
    const SkDraw&         fDraw;
    const SkPaint&        fPaint;
    const SkIRect         fClipBounds;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

SkScalerContextFlags SkDraw::scalerContextFlags() const {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (fDst.colorSpace() && fDst.colorSpace()->gammaIsLinear()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

//////////////////////////////////////////////////////////////////////////////

void SkDraw::blitARGB32Mask(const SkMask& mask, const SkPaint& paint) const {
    SkASSERT(SkMask::kARGB32_Format == mask.fFormat);
    SkBitmap bm;
    bm.installPixels(
            SkImageInfo::MakeN32Premul(mask.fBounds.width(), mask.fBounds.height()),
            (SkPMColor*)mask.fImage, mask.fRowBytes);

    this->drawSprite(bm, mask.fBounds.x(), mask.fBounds.y(), paint);
}

SkGlyphRunListPainter::PerMask SkDraw::drawOneMaskCreator(
        const SkPaint& paint, SkArenaAlloc* alloc) const {
    SkBlitter* blitter = SkBlitter::Choose(fDst, *fMatrix, paint, alloc, false);
    if (fCoverage != nullptr) {
        auto coverageBlitter = SkBlitter::Choose(*fCoverage, *fMatrix, SkPaint(), alloc, true);
        blitter = alloc->make<SkPairBlitter>(blitter, coverageBlitter);
    }

    auto wrapaper = alloc->make<SkAAClipBlitterWrapper>(*fRC, blitter);
    blitter = wrapaper->getBlitter();

    auto useRegion = fRC->isBW() && !fRC->isRect();

    if (useRegion) {
        return [this, blitter, &paint](const SkMask& mask, const SkGlyph&, SkPoint) {
            SkRegion::Cliperator clipper(fRC->bwRgn(), mask.fBounds);

            if (!clipper.done()) {
                if (SkMask::kARGB32_Format == mask.fFormat) {
                    this->blitARGB32Mask(mask, paint);
                } else {
                    const SkIRect& cr = clipper.rect();
                    do {
                        blitter->blitMask(mask, cr);
                        clipper.next();
                    } while (!clipper.done());
                }
            }
        };
    } else {
        SkIRect clipBounds = fRC->isBW() ? fRC->bwRgn().getBounds()
                                         : fRC->aaRgn().getBounds();
        return [this, blitter, clipBounds, &paint](const SkMask& mask, const SkGlyph&, SkPoint) {
            SkIRect  storage;
            const SkIRect* bounds = &mask.fBounds;

            // this extra test is worth it, assuming that most of the time it succeeds
            // since we can avoid writing to storage
            if (!clipBounds.containsNoEmptyCheck(mask.fBounds)) {
                if (!storage.intersectNoEmptyCheck(mask.fBounds, clipBounds)) {
                    return;
                }
                bounds = &storage;
            }

            if (SkMask::kARGB32_Format == mask.fFormat) {
                this->blitARGB32Mask(mask, paint);
            } else {
                blitter->blitMask(mask, *bounds);
            }
        };
    }
}

void SkDraw::drawGlyphRunList(
        const SkGlyphRunList& glyphRunList, SkGlyphRunListPainter* glyphPainter) const {

    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    SkMatrix renderMatrix{*fMatrix};
    auto perPathBuilder = [this, &renderMatrix]
            (const SkPaint& paint, SkScalar scaleMatrix, SkArenaAlloc*) {
        renderMatrix.setScale(scaleMatrix, scaleMatrix);
        auto perPath =
                [this, &renderMatrix, &paint]
                (const SkPath* path, const SkGlyph&, SkPoint position) {
            if (path != nullptr) {
                renderMatrix[SkMatrix::kMTransX] = position.fX;
                renderMatrix[SkMatrix::kMTransY] = position.fY;
                this->drawPath(*path, paint, &renderMatrix, false);
            }
        };
        return perPath;
    };

    auto perMaskBuilder = [this](const SkPaint& paint, SkArenaAlloc* alloc) {
        return this->drawOneMaskCreator(paint, alloc);
    };

    glyphPainter->drawForBitmapDevice(glyphRunList, *fMatrix, perMaskBuilder, perPathBuilder);
}

#if defined _WIN32
#pragma warning ( pop )
#endif

