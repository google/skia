/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextContext.h"
#include "GrContext.h"

#include "SkAutoKern.h"
#include "SkGlyphCache.h"
#include "GrFontScaler.h"

GrTextContext::GrTextContext(GrContext* context, const SkDeviceProperties& properties) :
                            fContext(context), fDeviceProperties(properties), fDrawTarget(NULL) {
}

void GrTextContext::init(const GrPaint& grPaint, const SkPaint& skPaint) {
    const GrClipData* clipData = fContext->getClip();

    SkRect devConservativeBound;
    clipData->fClipStack->getConservativeBounds(
                                     -clipData->fOrigin.fX,
                                     -clipData->fOrigin.fY,
                                     fContext->getRenderTarget()->width(),
                                     fContext->getRenderTarget()->height(),
                                     &devConservativeBound);

    devConservativeBound.roundOut(&fClipRect);

    fDrawTarget = fContext->getTextTarget();

    fPaint = grPaint;
    fSkPaint = skPaint;
}

//*** change to output positions?
void GrTextContext::MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                                const char text[], size_t byteLength, SkVector* stopVector) {
    SkFixed     x = 0, y = 0;
    const char* stop = text + byteLength;

    SkAutoKern  autokern;

    while (text < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

        x += autokern.adjust(glyph) + glyph.fAdvanceX;
        y += glyph.fAdvanceY;
    }
    stopVector->set(SkFixedToScalar(x), SkFixedToScalar(y));

    SkASSERT(text == stop);
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
