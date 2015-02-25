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
#include "SkGlyphCache.h"

GrTextContext::GrTextContext(GrContext* context, const SkDeviceProperties& properties) :
                            fFallbackTextContext(NULL),
                            fContext(context), fDeviceProperties(properties), fDrawTarget(NULL) {
}

GrTextContext::~GrTextContext() {
    SkDELETE(fFallbackTextContext);
}

void GrTextContext::init(GrRenderTarget* rt, const GrClip& clip, const GrPaint& grPaint,
                         const SkPaint& skPaint) {
    fClip = clip;

    fRenderTarget.reset(SkRef(rt));

    fClip.getConservativeBounds(fRenderTarget->width(), fRenderTarget->height(), &fClipRect);

    fDrawTarget = fContext->getTextTarget();

    fPaint = grPaint;
    fSkPaint = skPaint;
}

bool GrTextContext::drawText(GrRenderTarget* rt, const GrClip& clip, const GrPaint& paint,
                             const SkPaint& skPaint, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y) {
    if (!fContext->getTextTarget()) {
        return false;
    }

    GrTextContext* textContext = this;
    do {
        if (textContext->canDraw(skPaint, viewMatrix)) {
            textContext->onDrawText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, x, y);
            return true;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    return false;
}

bool GrTextContext::drawPosText(GrRenderTarget* rt, const GrClip& clip, const GrPaint& paint,
                                const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset) {
    if (!fContext->getTextTarget()) {
        return false;
    }

    GrTextContext* textContext = this;
    do {
        if (textContext->canDraw(skPaint, viewMatrix)) {
            textContext->onDrawPosText(rt, clip, paint, skPaint, viewMatrix, text, byteLength, pos,
                                       scalarsPerPosition, offset);
            return true;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    return false;
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
