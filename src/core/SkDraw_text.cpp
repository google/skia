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



void SkDraw::drawGlyphRunList(
        const SkGlyphRunList& glyphRunList, SkGlyphRunListPainter* glyphPainter) const {

    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    glyphPainter->drawForBitmapDevice(glyphRunList, *fMatrix, this);
}

void SkDraw::glyphPainterDrawPaths(
        const SkPaint& paint,
        SkSpan<const SkPoint> positions,
        const SkMatrix& scale,
        SkSpan<const SkPath*> paths) const {
    SkMatrix renderMatrix = scale;
    const SkPoint* positionCursor = positions.begin();
    for (auto path : paths) {
        auto position = *positionCursor++;
        if (path != nullptr) {
            renderMatrix[SkMatrix::kMTransX] = position.fX;
            renderMatrix[SkMatrix::kMTransY] = position.fY;
            this->drawPath(*path, paint, &renderMatrix, false);
        }
    }
}

void SkDraw::glyphPainterDrawMasks(
        const SkPaint& paint,
        SkSpan<const SkMask> masks) const {
    SkSTArenaAlloc<3332> alloc;

    SkBlitter* blitter = SkBlitter::Choose(fDst, *fMatrix, paint, &alloc, false);
    if (fCoverage != nullptr) {
        auto coverageBlitter = SkBlitter::Choose(*fCoverage, *fMatrix, SkPaint(), &alloc, true);
        blitter = alloc.make<SkPairBlitter>(blitter, coverageBlitter);
    }

    auto wrapaper = alloc.make<SkAAClipBlitterWrapper>(*fRC, blitter);
    blitter = wrapaper->getBlitter();

    auto useRegion = fRC->isBW() && !fRC->isRect();

    if (useRegion) {
        for (const SkMask& mask : masks) {
            if (mask.fImage != nullptr) {
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
            }
        }
    } else {
        SkIRect clipBounds = fRC->isBW() ? fRC->bwRgn().getBounds()
                                         : fRC->aaRgn().getBounds();
        for (const SkMask& mask : masks) {
            if (mask.fImage != nullptr) {
                SkIRect storage;
                const SkIRect* bounds = &mask.fBounds;

                // this extra test is worth it, assuming that most of the time it succeeds
                // since we can avoid writing to storage
                if (!clipBounds.containsNoEmptyCheck(mask.fBounds)) {
                    if (!storage.intersectNoEmptyCheck(mask.fBounds, clipBounds)) {
                        continue;
                    }
                    bounds = &storage;
                }

                if (SkMask::kARGB32_Format == mask.fFormat) {
                    this->blitARGB32Mask(mask, paint);
                } else {
                    blitter->blitMask(mask, *bounds);
                }
            }
        }
    }
}

#if defined _WIN32
#pragma warning ( pop )
#endif

