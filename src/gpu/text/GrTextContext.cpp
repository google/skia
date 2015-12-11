/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextContext.h"
#include "GrContext.h"
#include "GrFontScaler.h"
#include "GrTextUtils.h"

#include "SkDrawFilter.h"
#include "SkGlyphCache.h"
#include "SkGrPriv.h"
#include "SkTextBlobRunIterator.h"

GrTextContext::GrTextContext(GrContext* context, const SkSurfaceProps& surfaceProps)
    : fFallbackTextContext(nullptr)
    , fContext(context)
    , fSurfaceProps(surfaceProps) {
}

GrTextContext::~GrTextContext() {
    delete fFallbackTextContext;
}

void GrTextContext::drawText(GrDrawContext* dc,
                             const GrClip& clip, const GrPaint& paint,
                             const SkPaint& skPaint, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y, const SkIRect& clipBounds) {
    if (fContext->abandoned()) {
        return;
    }

    GrTextContext* textContext = this;
    do {
        if (textContext->canDraw(skPaint, viewMatrix)) {
            textContext->onDrawText(dc, clip, paint, skPaint, viewMatrix,
                                    text, byteLength, x, y, clipBounds);
            return;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    // fall back to drawing as a path
    GrTextUtils::DrawTextAsPath(fContext, dc, clip, skPaint, viewMatrix, text, byteLength, x, y,
                                clipBounds);
}

void GrTextContext::drawPosText(GrDrawContext* dc,
                                const GrClip& clip, const GrPaint& paint,
                                const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset, const SkIRect& clipBounds) {
    if (fContext->abandoned()) {
        return;
    }

    GrTextContext* textContext = this;
    do {
        if (textContext->canDraw(skPaint, viewMatrix)) {
            textContext->onDrawPosText(dc, clip, paint, skPaint, viewMatrix,
                                       text, byteLength, pos,
                                       scalarsPerPosition, offset, clipBounds);
            return;
        }
        textContext = textContext->fFallbackTextContext;
    } while (textContext);

    // fall back to drawing as a path
    GrTextUtils::DrawPosTextAsPath(fContext, dc, fSurfaceProps, clip, skPaint, viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset, clipBounds);
}

bool GrTextContext::ShouldDisableLCD(const SkPaint& paint) {
    if (!SkXfermode::AsMode(paint.getXfermode(), nullptr) ||
        paint.getMaskFilter() ||
        paint.getRasterizer() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style)
    {
        return true;
    }
    return false;
}

uint32_t GrTextContext::FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint) {
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

void GrTextContext::drawTextBlob(GrDrawContext* dc,
                                 const GrClip& clip, const SkPaint& skPaint,
                                 const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                 SkScalar x, SkScalar y,
                                 SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    SkPaint runPaint = skPaint;

    SkTextBlobRunIterator it(blob);
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

        runPaint.setFlags(FilterTextFlags(fSurfaceProps, runPaint));

        GrPaint grPaint;
        if (!SkPaintToGrPaint(fContext, runPaint, viewMatrix, &grPaint)) {
            return;
        }

        switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning:
            this->drawText(dc, clip, grPaint, runPaint, viewMatrix, (const char *)it.glyphs(),
                           textLen, x + offset.x(), y + offset.y(), clipBounds);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            this->drawPosText(dc, clip, grPaint, runPaint, viewMatrix, (const char*)it.glyphs(),
                              textLen, it.pos(), 1, SkPoint::Make(x, y + offset.y()), clipBounds);
            break;
        case SkTextBlob::kFull_Positioning:
            this->drawPosText(dc, clip, grPaint, runPaint, viewMatrix, (const char*)it.glyphs(),
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

static void GlyphCacheAuxProc(void* data) {
    GrFontScaler* scaler = (GrFontScaler*)data;
    SkSafeUnref(scaler);
}

GrFontScaler* GrTextContext::GetGrFontScaler(SkGlyphCache* cache) {
    void* auxData;
    GrFontScaler* scaler = nullptr;

    if (cache->getAuxProcData(GlyphCacheAuxProc, &auxData)) {
        scaler = (GrFontScaler*)auxData;
    }
    if (nullptr == scaler) {
        scaler = new GrFontScaler(cache);
        cache->setAuxProc(GlyphCacheAuxProc, scaler);
    }

    return scaler;
}
