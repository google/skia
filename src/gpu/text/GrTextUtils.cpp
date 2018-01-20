/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextUtils.h"
#include "GrContext.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGr.h"
#include "SkPaint.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextMapStateProc.h"
#include "SkTextToPathIter.h"

void GrTextUtils::Paint::initFilteredColor() {
    // This mirrors the logic in skpaint_to_grpaint_impl for handling paint colors
    if (fDstColorSpaceInfo->colorSpace()) {
        GrColor4f filteredColor =
                SkColorToUnpremulGrColor4f(fPaint->getColor(), *fDstColorSpaceInfo);
        if (fPaint->getColorFilter()) {
            filteredColor = GrColor4f::FromSkColor4f(
                fPaint->getColorFilter()->filterColor4f(filteredColor.toSkColor4f()));
        }
        fFilteredPremulColor = filteredColor.premul().toGrColor();
    } else {
        SkColor filteredSkColor = fPaint->getColor();
        if (fPaint->getColorFilter()) {
            filteredSkColor = fPaint->getColorFilter()->filterColor(filteredSkColor);
        }
        fFilteredPremulColor = SkColorToPremulGrColor(filteredSkColor);
    }
}

bool GrTextUtils::RunPaint::modifyForRun(const SkTextBlobRunIterator& run) {
    if (!fModifiedPaint.isValid()) {
        fModifiedPaint.init(fOriginalPaint->skPaint());
        fPaint = fModifiedPaint.get();
    } else if (fFilter) {
        // We have to reset before applying the run because the filter could have arbitrary
        // changed the paint.
        *fModifiedPaint.get() = fOriginalPaint->skPaint();
    }
    run.applyFontToPaint(fModifiedPaint.get());

    if (fFilter) {
        if (!fFilter->filter(fModifiedPaint.get(), SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            return false;
        }
        // The draw filter could have changed either the paint color or color filter.
        this->initFilteredColor();
    }
    fModifiedPaint.get()->setFlags(FilterTextFlags(fProps, *fModifiedPaint.get()));
    return true;
}

uint32_t GrTextUtils::FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint) {
    uint32_t flags = paint.getFlags();

    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        return flags;
    }

    if (kUnknown_SkPixelGeometry == surfaceProps.pixelGeometry() || ShouldDisableLCD(paint)) {
        flags &= ~SkPaint::kLCDRenderText_Flag;
        flags |= SkPaint::kGenA8FromLCD_Flag;
    }

    return flags;
}

bool GrTextUtils::ShouldDisableLCD(const SkPaint& paint) {
    return paint.getMaskFilter() || paint.getRasterizer() || paint.getPathEffect() ||
           paint.isFakeBoldText() || paint.getStyle() != SkPaint::kFill_Style;
}

void GrTextUtils::DrawBigText(GrContext* context, GrTextUtils::Target* target,
                              const GrClip& clip, const SkPaint& paint,
                              const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                              SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    if (!paint.countText(text, byteLength)) {
        return;
    }
    SkTextToPathIter iter(text, byteLength, paint, true);

    SkMatrix    matrix;
    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    matrix.postTranslate(x, y);

    const SkPath* iterPath;
    SkScalar xpos, prevXPos = 0;

    while (iter.next(&iterPath, &xpos)) {
        matrix.postTranslate(xpos - prevXPos, 0);
        if (iterPath) {
            const SkPaint& pnt = iter.getPaint();
            target->drawPath(clip, *iterPath, pnt, viewMatrix, &matrix, clipBounds);
        }
        prevXPos = xpos;
    }
}

void GrTextUtils::DrawBigPosText(GrContext* context, GrTextUtils::Target* target,
                                 const SkSurfaceProps& props, const GrClip& clip,
                                 const SkPaint& origPaint, const SkMatrix& viewMatrix,
                                 const char text[], size_t byteLength, const SkScalar pos[],
                                 int scalarsPerPosition, const SkPoint& offset,
                                 const SkIRect& clipBounds) {
    if (!origPaint.countText(text, byteLength)) {
        return;
    }
    // setup our std paint, in hopes of getting hits in the cache
    SkPaint paint(origPaint);
    SkScalar matrixScale = paint.setupForAsPaths();

    SkMatrix matrix;
    matrix.setScale(matrixScale, matrixScale);

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    paint.setStyle(SkPaint::kFill_Style);
    paint.setPathEffect(nullptr);

    SkPaint::GlyphCacheProc    glyphCacheProc = SkPaint::GetGlyphCacheProc(paint.getTextEncoding(),
                                                                           paint.isDevKernText(),
                                                                           true);
    SkAutoGlyphCache           autoCache(paint, &props, nullptr);
    SkGlyphCache*              cache = autoCache.getCache();

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(paint.getTextAlign());
    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);

    // Now restore the original settings, so we "draw" with whatever style/stroking.
    paint.setStyle(origPaint.getStyle());
    paint.setPathEffect(origPaint.refPathEffect());

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text);
        if (glyph.fWidth) {
            const SkPath* path = cache->findPath(glyph);
            if (path) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkPoint loc;
                alignProc(tmsLoc, glyph, &loc);

                matrix[SkMatrix::kMTransX] = loc.fX;
                matrix[SkMatrix::kMTransY] = loc.fY;
                target->drawPath(clip, *path, paint, viewMatrix, &matrix, clipBounds);
            }
        }
        pos += scalarsPerPosition;
    }
}
