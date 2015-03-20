/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextContext.h"
#include "GrContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"

#include "SkAutoKern.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkTextMapStateProc.h"
#include "SkTextToPathIter.h"

GrTextContext::GrTextContext(GrContext* context, SkGpuDevice* gpuDevice,
                             const SkDeviceProperties& properties)
    : fFallbackTextContext(NULL)
    , fContext(context)
    , fGpuDevice(gpuDevice)
    , fDeviceProperties(properties)
    , fDrawTarget(NULL) {
}

GrTextContext::~GrTextContext() {
    SkDELETE(fFallbackTextContext);
}

void GrTextContext::init(GrRenderTarget* rt, const GrClip& clip, const GrPaint& grPaint,
                         const SkPaint& skPaint, const SkIRect& regionClipBounds) {
    fClip = clip;

    fRenderTarget.reset(SkRef(rt));

    fRegionClipBounds = regionClipBounds;
    fClip.getConservativeBounds(fRenderTarget->width(), fRenderTarget->height(), &fClipRect);

    fDrawTarget = fContext->getTextTarget();

    fPaint = grPaint;
    fSkPaint = skPaint;
}

void GrTextContext::drawText(GrRenderTarget* rt, const GrClip& clip, const GrPaint& paint,
                             const SkPaint& skPaint, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    if (!fContext->getTextTarget()) {
        return;
    }

    GrTextContext* textContext = this;
    do {
        if (textContext->canDraw(skPaint, viewMatrix)) {
            textContext->onDrawText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, x, y,
                                    clipBounds);
            return;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    // fall back to drawing as a path
    this->drawTextAsPath(skPaint, viewMatrix, text, byteLength, x, y, clipBounds);
}

void GrTextContext::drawPosText(GrRenderTarget* rt, const GrClip& clip, const GrPaint& paint,
                                const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset, const SkIRect& clipBounds) {
    if (!fContext->getTextTarget()) {
        return;
    }

    GrTextContext* textContext = this;
    do {
        if (textContext->canDraw(skPaint, viewMatrix)) {
            textContext->onDrawPosText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, pos,
                                       scalarsPerPosition, offset, clipBounds);
            return;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    // fall back to drawing as a path
    this->drawPosTextAsPath(skPaint, viewMatrix, text, byteLength, pos, scalarsPerPosition, offset,
                            clipBounds);
}

void GrTextContext::drawTextAsPath(const SkPaint& skPaint, const SkMatrix& viewMatrix,
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
            fGpuDevice->internalDrawPath(*iterPath, pnt, viewMatrix, &matrix, clipBounds, false);
        }
        prevXPos = xpos;
    }
}

void GrTextContext::drawPosTextAsPath(const SkPaint& origPaint, const SkMatrix& viewMatrix,
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
    paint.setPathEffect(NULL);

    SkDrawCacheProc     glyphCacheProc = paint.getDrawCacheProc();
    SkAutoGlyphCache    autoCache(paint, NULL, NULL);
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
                fGpuDevice->internalDrawPath(*path, paint, viewMatrix, &matrix, clipBounds, false);
            }
        }
        pos += scalarsPerPosition;
    }
}

//*** change to output positions?
int GrTextContext::MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                                const char text[], size_t byteLength, SkVector* stopVector) {
    SkFixed     x = 0, y = 0;
    const char* stop = text + byteLength;

    SkAutoKern  autokern;

    int numGlyphs = 0;
    while (text < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

        x += autokern.adjust(glyph) + glyph.fAdvanceX;
        y += glyph.fAdvanceY;
        ++numGlyphs;
    }
    stopVector->set(SkFixedToScalar(x), SkFixedToScalar(y));

    SkASSERT(text == stop);
    
    return numGlyphs;
}

static void GlyphCacheAuxProc(void* data) {
    GrFontScaler* scaler = (GrFontScaler*)data;
    SkSafeUnref(scaler);
}

GrFontScaler* GrTextContext::GetGrFontScaler(SkGlyphCache* cache) {
    void* auxData;
    GrFontScaler* scaler = NULL;

    if (cache->getAuxProcData(GlyphCacheAuxProc, &auxData)) {
        scaler = (GrFontScaler*)auxData;
    }
    if (NULL == scaler) {
        scaler = SkNEW_ARGS(GrFontScaler, (cache));
        cache->setAuxProc(GlyphCacheAuxProc, scaler);
    }

    return scaler;
}
