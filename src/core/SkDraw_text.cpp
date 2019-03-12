/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDraw.h"
#include "SkFontPriv.h"
#include "SkPaintPriv.h"
#include "SkRasterClip.h"
#include "SkScalerContext.h"
#include "SkStrike.h"
#include "SkUtils.h"

bool SkDraw::ShouldDrawTextAsPaths(const SkFont& font, const SkPaint& paint,
                                   const SkMatrix& ctm, SkScalar sizeLimit) {
    // hairline glyphs are fast enough so we don't need to cache them
    if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
        return true;
    }

    // we don't cache perspective
    if (ctm.hasPerspective()) {
        return true;
    }

    return SkFontPriv::TooBigToUseCache(ctm, SkFontPriv::MakeTextMatrix(font), sizeLimit);
}

// disable warning : local variable used without having been initialized
#if defined _WIN32
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

void SkDraw::paintMasks(SkSpan<const SkMask> masks, const SkPaint& paint) const {

    // The size used for a typical blitter.
    SkSTArenaAlloc<3308> alloc;
    SkBlitter* blitter = SkBlitter::Choose(fDst, *fMatrix, paint, &alloc, false);
    if (fCoverage) {
        blitter = alloc.make<SkPairBlitter>(
                blitter,
                SkBlitter::Choose(*fCoverage, *fMatrix, SkPaint(), &alloc, true));
    }

    SkAAClipBlitterWrapper wrapper{*fRC, blitter};
    blitter = wrapper.getBlitter();

    bool useRegion = fRC->isBW() && !fRC->isRect();

    if (useRegion) {
        for (const SkMask& mask : masks) {
            SkRegion::Cliperator clipper(fRC->bwRgn(), mask.fBounds);

            if (!clipper.done()) {
                if (SkMask::kARGB32_Format == mask.fFormat) {
                    SkBitmap bm;
                    bm.installPixels(SkImageInfo::MakeN32Premul(mask.fBounds.size()),
                                     mask.fImage,
                                     mask.fRowBytes);
                    this->drawSprite(bm, mask.fBounds.x(), mask.fBounds.y(), paint);
                } else {
                    const SkIRect& cr = clipper.rect();
                    do {
                        blitter->blitMask(mask, cr);
                        clipper.next();
                    } while (!clipper.done());
                }
            }
        }
    } else {
        SkIRect clipBounds = fRC->isBW() ? fRC->bwRgn().getBounds()
                                         : fRC->aaRgn().getBounds();
        for (const SkMask& mask : masks) {
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
                SkBitmap bm;
                bm.installPixels(SkImageInfo::MakeN32Premul(mask.fBounds.size()),
                                 mask.fImage,
                                 mask.fRowBytes);
                this->drawSprite(bm, mask.fBounds.x(), mask.fBounds.y(), paint);
            } else {
                blitter->blitMask(mask, *bounds);
            }
        }
    }
}

void SkDraw::paintPaths(SkSpan<const SkPathPos> pathsAndPositions,
                        SkScalar scale,
                        const SkPaint& paint) const {
    for (const auto& pathAndPos : pathsAndPositions) {
        SkMatrix m;
        SkPoint position = pathAndPos.position;
        m.setScaleTranslate(scale, scale, position.x(), position.y());
        this->drawPath(*pathAndPos.path, paint, &m, false);
    }
}

void SkDraw::drawGlyphRunList(const SkGlyphRunList& glyphRunList,
                              SkGlyphRunListPainter* glyphPainter) const {

    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    glyphPainter->drawForBitmapDevice(glyphRunList, *fMatrix, this);
}

#if defined _WIN32
#pragma warning ( pop )
#endif

