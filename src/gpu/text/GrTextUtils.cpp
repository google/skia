/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextUtils.h"

#include "GrAtlasTextBlob.h"
#include "GrBatchFontCache.h"
#include "GrBlurUtils.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrTextContext.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkTextMapStateProc.h"
#include "SkTextToPathIter.h"

void GrTextUtils::DrawBmpText(GrAtlasTextBlob* blob, int runIndex,
                              GrBatchFontCache* fontCache,
                              const SkSurfaceProps& props, const SkPaint& skPaint,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              const char text[], size_t byteLength,
                              SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    GrBatchTextStrike* currStrike = nullptr;

    // Get GrFontScaler from cache
    SkGlyphCache* cache = blob->setupCache(runIndex, props, skPaint, &viewMatrix, false);
    GrFontScaler* fontScaler = GrTextContext::GetGrFontScaler(cache);

    SkFindAndPlaceGlyph::ProcessText(
        skPaint.getTextEncoding(), text, byteLength,
        {x, y}, viewMatrix, skPaint.getTextAlign(),
        cache,
        [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
            position += rounding;
            BmpAppendGlyph(
                blob, runIndex, fontCache, &currStrike, glyph,
                SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                color, fontScaler);
        }
    );

    SkGlyphCache::AttachCache(cache);
}

void GrTextUtils::DrawBmpPosText(GrAtlasTextBlob* blob, int runIndex,
                                 GrBatchFontCache* fontCache,
                                 const SkSurfaceProps& props, const SkPaint& skPaint,
                                 GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const char text[], size_t byteLength,
                                 const SkScalar pos[], int scalarsPerPosition,
                                 const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    GrBatchTextStrike* currStrike = nullptr;

    // Get GrFontScaler from cache
    SkGlyphCache* cache = blob->setupCache(runIndex, props, skPaint, &viewMatrix, false);
    GrFontScaler* fontScaler = GrTextContext::GetGrFontScaler(cache);

    SkFindAndPlaceGlyph::ProcessPosText(
        skPaint.getTextEncoding(), text, byteLength,
        offset, viewMatrix, pos, scalarsPerPosition,
        skPaint.getTextAlign(), cache,
        [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
            position += rounding;
            BmpAppendGlyph(
                blob, runIndex, fontCache, &currStrike, glyph,
                SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                color, fontScaler);
        }
    );

    SkGlyphCache::AttachCache(cache);
}

void GrTextUtils::BmpAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                 GrBatchFontCache* fontCache,
                                 GrBatchTextStrike** strike, const SkGlyph& skGlyph,
                                 int vx, int vy, GrColor color, GrFontScaler* scaler) {
    if (!*strike) {
        *strike = fontCache->getStrike(scaler);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kCoverage_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, scaler);
    if (!glyph) {
        return;
    }

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    // keep them as ints until we've done the clip-test
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);

    blob->appendGlyph(runIndex, r, color, *strike, glyph, scaler, skGlyph,
                      SkIntToScalar(vx), SkIntToScalar(vy), 1.0f, false);
}

void GrTextUtils::DrawTextAsPath(GrContext* context, GrDrawContext* dc,
                                 const GrClip& clip,
                                 const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                 const char text[], size_t byteLength, SkScalar x, SkScalar y,
                                 const SkIRect& clipBounds) {
    SkTextToPathIter iter(text, byteLength, skPaint, true);

    SkMatrix    matrix;
    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    matrix.postTranslate(x, y);

    const SkPath* iterPath;
    SkScalar xpos, prevXPos = 0;

    while (iter.next(&iterPath, &xpos)) {
        matrix.postTranslate(xpos - prevXPos, 0);
        if (iterPath) {
            const SkPaint& pnt = iter.getPaint();
            GrBlurUtils::drawPathWithMaskFilter(context, dc, clip, *iterPath,
                                                pnt, viewMatrix, &matrix, clipBounds, false);
        }
        prevXPos = xpos;
    }
}

void GrTextUtils::DrawPosTextAsPath(GrContext* context,
                                    GrDrawContext* dc,
                                    const SkSurfaceProps& props,
                                    const GrClip& clip,
                                    const SkPaint& origPaint, const SkMatrix& viewMatrix,
                                    const char text[], size_t byteLength,
                                    const SkScalar pos[], int scalarsPerPosition,
                                    const SkPoint& offset, const SkIRect& clipBounds) {
    // setup our std paint, in hopes of getting hits in the cache
    SkPaint paint(origPaint);
    SkScalar matrixScale = paint.setupForAsPaths();

    SkMatrix matrix;
    matrix.setScale(matrixScale, matrixScale);

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    paint.setStyle(SkPaint::kFill_Style);
    paint.setPathEffect(nullptr);

    SkDrawCacheProc     glyphCacheProc = paint.getDrawCacheProc();
    SkAutoGlyphCache    autoCache(paint, &props, nullptr);
    SkGlyphCache*       cache = autoCache.getCache();

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(paint.getTextAlign());
    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);

    // Now restore the original settings, so we "draw" with whatever style/stroking.
    paint.setStyle(origPaint.getStyle());
    paint.setPathEffect(origPaint.getPathEffect());

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);
        if (glyph.fWidth) {
            const SkPath* path = cache->findPath(glyph);
            if (path) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkPoint loc;
                alignProc(tmsLoc, glyph, &loc);

                matrix[SkMatrix::kMTransX] = loc.fX;
                matrix[SkMatrix::kMTransY] = loc.fY;
                GrBlurUtils::drawPathWithMaskFilter(context, dc, clip, *path, paint,
                                                    viewMatrix, &matrix, clipBounds, false);
            }
        }
        pos += scalarsPerPosition;
    }
}
