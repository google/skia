/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "src/core/SkDraw.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkUtils.h"
#include <climits>

// disable warning : local variable used without having been initialized
#if defined _WIN32
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

static bool check_glyph_position(SkPoint position) {
    // Prevent glyphs from being drawn outside of or straddling the edge of device space.
    // Comparisons written a little weirdly so that NaN coordinates are treated safely.
    auto gt = [](float a, int b) { return !(a <= (float)b); };
    auto lt = [](float a, int b) { return !(a >= (float)b); };
    return !(gt(position.fX, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
             lt(position.fX, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)) ||
             gt(position.fY, INT_MAX - (INT16_MAX + SkTo<int>(UINT16_MAX))) ||
             lt(position.fY, INT_MIN - (INT16_MIN + 0 /*UINT16_MIN*/)));
}

void SkDraw::paintMasks(SkDrawableGlyphBuffer* drawables, const SkPaint& paint) const {

    // The size used for a typical blitter.
    SkSTArenaAlloc<3308> alloc;
    SkBlitter* blitter =
            SkBlitter::Choose(fDst, *fMatrixProvider, paint, &alloc, false, fRC->clipShader());
    if (fCoverage) {
        blitter = alloc.make<SkPairBlitter>(
                blitter,
                SkBlitter::Choose(
                        *fCoverage, *fMatrixProvider, SkPaint(), &alloc, true, fRC->clipShader()));
    }

    SkAAClipBlitterWrapper wrapper{*fRC, blitter};
    blitter = wrapper.getBlitter();

    bool useRegion = fRC->isBW() && !fRC->isRect();

    if (useRegion) {
        for (auto [variant, pos] : drawables->drawable()) {
            SkGlyph* glyph = variant.glyph();
            if (check_glyph_position(pos)) {
                SkMask mask = glyph->mask(pos);

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
        }
    } else {
        SkIRect clipBounds = fRC->isBW() ? fRC->bwRgn().getBounds()
                                         : fRC->aaRgn().getBounds();
        for (auto [variant, pos] : drawables->drawable()) {
            SkGlyph* glyph = variant.glyph();
            if (check_glyph_position(pos)) {
                SkMask mask = glyph->mask(pos);
                SkIRect storage;
                const SkIRect* bounds = &mask.fBounds;

                // this extra test is worth it, assuming that most of the time it succeeds
                // since we can avoid writing to storage
                if (!clipBounds.containsNoEmptyCheck(mask.fBounds)) {
                    if (!storage.intersect(mask.fBounds, clipBounds)) {
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
}

void SkDraw::paintPaths(SkDrawableGlyphBuffer* drawables,
                        SkScalar scale,
                        SkPoint origin,
                        const SkPaint& paint) const {
    for (auto [variant, pos] : drawables->drawable()) {
        const SkPath* path = variant.path();
        SkMatrix m;
        SkPoint translate = origin + pos;
        m.setScaleTranslate(scale, scale, translate.x(), translate.y());
        this->drawPath(*path, paint, &m, false);
    }
}

void SkDraw::drawGlyphRunList(const SkGlyphRunList& glyphRunList,
                              SkGlyphRunListPainter* glyphPainter) const {

    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    glyphPainter->drawForBitmapDevice(glyphRunList, fMatrixProvider->localToDevice(), this);
}

#if defined _WIN32
#pragma warning ( pop )
#endif

