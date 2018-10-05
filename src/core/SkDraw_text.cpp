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

