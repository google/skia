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
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkTextBlob.h"
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
        if (textContext->canDraw(rt, clip, paint, skPaint, viewMatrix)) {
            textContext->onDrawText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, x, y,
                                    clipBounds);
            return;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    // fall back to drawing as a path
    SkASSERT(fGpuDevice);
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
        if (textContext->canDraw(rt, clip, paint, skPaint, viewMatrix)) {
            textContext->onDrawPosText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, pos,
                                       scalarsPerPosition, offset, clipBounds);
            return;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    // fall back to drawing as a path
    SkASSERT(fGpuDevice);
    this->drawPosTextAsPath(skPaint, viewMatrix, text, byteLength, pos, scalarsPerPosition, offset,
                            clipBounds);
}

void GrTextContext::drawTextBlob(GrRenderTarget* rt, const GrClip& clip, const SkPaint& skPaint,
                                 const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                 SkScalar x, SkScalar y,
                                 SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    SkPaint runPaint = skPaint;

    SkTextBlob::RunIterator it(blob);
    for (;!it.done(); it.next()) {
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = skPaint;
            continue;
        }

        runPaint.setFlags(fGpuDevice->filterTextFlags(runPaint));

        GrPaint grPaint;
        if (!SkPaint2GrPaint(fContext, fRenderTarget, runPaint, viewMatrix, true, &grPaint)) {
            return;
        }

        switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning:
            this->drawText(rt, clip, grPaint, runPaint, viewMatrix, (const char *)it.glyphs(),
                           textLen, x + offset.x(), y + offset.y(), clipBounds);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            this->drawPosText(rt, clip, grPaint, runPaint, viewMatrix, (const char*)it.glyphs(),
                              textLen, it.pos(), 1, SkPoint::Make(x, y + offset.y()), clipBounds);
            break;
        case SkTextBlob::kFull_Positioning:
            this->drawPosText(rt, clip, grPaint, runPaint, viewMatrix, (const char*)it.glyphs(),
                              textLen, it.pos(), 2, SkPoint::Make(x, y), clipBounds);
            break;
        default:
            SkFAIL("unhandled positioning mode");
        }

        if (drawFilter) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = skPaint;
        }
    }
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
