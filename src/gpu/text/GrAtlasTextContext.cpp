/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrTextBlobCache.h"
#include "GrTextUtils.h"

#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkGrPriv.h"

GrAtlasTextContext::GrAtlasTextContext()
    : fDistanceAdjustTable(new GrDistanceFieldAdjustTable) {
}


GrAtlasTextContext* GrAtlasTextContext::Create() {
    return new GrAtlasTextContext();
}

bool GrAtlasTextContext::canDraw(const SkPaint& skPaint,
                                 const SkMatrix& viewMatrix,
                                 const SkSurfaceProps& props,
                                 const GrShaderCaps& shaderCaps) {
    return GrTextUtils::CanDrawAsDistanceFields(skPaint, viewMatrix, props, shaderCaps) ||
           !SkDraw::ShouldDrawTextAsPaths(skPaint, viewMatrix);
}

GrColor GrAtlasTextContext::ComputeCanonicalColor(const SkPaint& paint, bool lcd) {
    GrColor canonicalColor = paint.computeLuminanceColor();
    if (lcd) {
        // This is the correct computation, but there are tons of cases where LCD can be overridden.
        // For now we just regenerate if any run in a textblob has LCD.
        // TODO figure out where all of these overrides are and see if we can incorporate that logic
        // at a higher level *OR* use sRGB
        SkASSERT(false);
        //canonicalColor = SkMaskGamma::CanonicalColor(canonicalColor);
    } else {
        // A8, though can have mixed BMP text but it shouldn't matter because BMP text won't have
        // gamma corrected masks anyways, nor color
        U8CPU lum = SkComputeLuminance(SkColorGetR(canonicalColor),
                                       SkColorGetG(canonicalColor),
                                       SkColorGetB(canonicalColor));
        // reduce to our finite number of bits
        canonicalColor = SkMaskGamma::CanonicalColor(SkColorSetRGB(lum, lum, lum));
    }
    return canonicalColor;
}

SkPaint::FakeGamma GrAtlasTextContext::ComputeFakeGamma(GrDrawContext* dc, const GrPaint& grPaint) {
    // If we're rendering to an sRGB render target, and we aren't forcing sRGB blending off,
    // then we can disable the gamma hacks. Otherwise, leave them on:
    if (GrPixelConfigIsSRGB(dc->accessRenderTarget()->config()) &&
        !grPaint.getDisableOutputConversionToSRGB()) {
        return SkPaint::FakeGamma::Off;
    } else {
        return SkPaint::FakeGamma::On;
    }
}

// TODO if this function ever shows up in profiling, then we can compute this value when the
// textblob is being built and cache it.  However, for the time being textblobs mostly only have 1
// run so this is not a big deal to compute here.
bool GrAtlasTextContext::HasLCD(const SkTextBlob* blob) {
    SkTextBlobRunIterator it(blob);
    for (; !it.done(); it.next()) {
        if (it.isLCD()) {
            return true;
        }
    }
    return false;
}

void GrAtlasTextContext::drawTextBlob(GrContext* context, GrDrawContext* dc,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      const SkMatrix& viewMatrix,
                                      const SkSurfaceProps& props, const SkTextBlob* blob,
                                      SkScalar x, SkScalar y,
                                      SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (context->abandoned()) {
        return;
    }

    SkAutoTUnref<GrAtlasTextBlob> cacheBlob;
    SkMaskFilter::BlurRec blurRec;
    GrAtlasTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() ||
                      (mf && !mf->asABlur(&blurRec)) ||
                      drawFilter);

    GrTextBlobCache* cache = context->getTextBlobCache();
    if (canCache) {
        bool hasLCD = HasLCD(blob);

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? props.pixelGeometry() :
                                                 kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                          ComputeCanonicalColor(skPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = blob->uniqueID();
        key.fStyle = skPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        cacheBlob.reset(SkSafeRef(cache->find(key)));
    }

    // Though for the time being runs in the textblob can override the paint, they only touch font
    // info.
    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, skPaint, viewMatrix, props.allowSRGBInputs(), &grPaint)) {
        return;
    }

    SkPaint::FakeGamma fakeGamma = ComputeFakeGamma(dc, grPaint);

    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(skPaint, grPaint.getColor(), blurRec, viewMatrix, x, y)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            cache->remove(cacheBlob);
            cacheBlob.reset(SkRef(cache->createCachedBlob(blob, key, blurRec, skPaint)));
            RegenerateTextBlob(cacheBlob, context->getBatchFontCache(),
                               *context->caps()->shaderCaps(), skPaint, grPaint.getColor(),
                               fakeGamma, viewMatrix, props,
                               blob, x, y, drawFilter);
        } else {
            cache->makeMRU(cacheBlob);

            if (CACHE_SANITY_CHECK) {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                SkAutoTUnref<GrAtlasTextBlob> sanityBlob(cache->createBlob(glyphCount, runCount));
                sanityBlob->setupKey(key, blurRec, skPaint);
                RegenerateTextBlob(sanityBlob, context->getBatchFontCache(),
                                   *context->caps()->shaderCaps(), skPaint,
                                   grPaint.getColor(), fakeGamma, viewMatrix, props,
                                   blob, x, y, drawFilter);
                GrAtlasTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob.reset(SkRef(cache->createCachedBlob(blob, key, blurRec, skPaint)));
        } else {
            cacheBlob.reset(cache->createBlob(blob));
        }
        RegenerateTextBlob(cacheBlob, context->getBatchFontCache(),
                           *context->caps()->shaderCaps(), skPaint, grPaint.getColor(),
                           fakeGamma, viewMatrix, props,
                           blob, x, y, drawFilter);
    }

    cacheBlob->flushCached(context, dc, blob, props, fDistanceAdjustTable, skPaint,
                           grPaint, drawFilter, clip, viewMatrix, clipBounds, x, y);
}

void GrAtlasTextContext::RegenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            GrBatchFontCache* fontCache,
                                            const GrShaderCaps& shaderCaps,
                                            const SkPaint& skPaint, GrColor color,
                                            SkPaint::FakeGamma fakeGamma,
                                            const SkMatrix& viewMatrix,
                                            const SkSurfaceProps& props,
                                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                                            SkDrawFilter* drawFilter) {
    cacheBlob->initReusableBlob(color, viewMatrix, x, y);

    // Regenerate textblob
    SkPaint runPaint = skPaint;
    SkTextBlobRunIterator it(blob);
    for (int run = 0; !it.done(); it.next(), run++) {
        int glyphCount = it.glyphCount();
        size_t textLen = glyphCount * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = skPaint;
            continue;
        }

        runPaint.setFlags(GrTextUtils::FilterTextFlags(props, runPaint));

        cacheBlob->push_back_run(run);

        if (GrTextUtils::CanDrawAsDistanceFields(runPaint, viewMatrix, props, shaderCaps)) {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    GrTextUtils::DrawDFText(cacheBlob, run, fontCache,
                                            props, runPaint, color, fakeGamma, viewMatrix,
                                            (const char *)it.glyphs(), textLen,
                                            x + offset.x(), y + offset.y());
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y + offset.y());
                    GrTextUtils::DrawDFPosText(cacheBlob, run, fontCache,
                                               props, runPaint, color, fakeGamma, viewMatrix,
                                               (const char*)it.glyphs(), textLen, it.pos(),
                                               1, dfOffset);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y);
                    GrTextUtils::DrawDFPosText(cacheBlob, run,  fontCache,
                                               props, runPaint, color, fakeGamma, viewMatrix,
                                               (const char*)it.glyphs(), textLen, it.pos(),
                                               2, dfOffset);
                    break;
                }
            }
        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            cacheBlob->setRunDrawAsPaths(run);
        } else {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning:
                    GrTextUtils::DrawBmpText(cacheBlob, run, fontCache,
                                             props, runPaint, color, fakeGamma, viewMatrix,
                                             (const char *)it.glyphs(), textLen,
                                             x + offset.x(), y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fontCache,
                                                props, runPaint, color, fakeGamma, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 1,
                                                SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fontCache,
                                                props, runPaint, color, fakeGamma, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 2,
                                                SkPoint::Make(x, y));
                    break;
            }
        }

        if (drawFilter) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = skPaint;
        }
    }
}

inline GrAtlasTextBlob*
GrAtlasTextContext::CreateDrawTextBlob(GrTextBlobCache* blobCache,
                                       GrBatchFontCache* fontCache,
                                       const GrShaderCaps& shaderCaps,
                                       const GrPaint& paint,
                                       const SkPaint& skPaint,
                                       SkPaint::FakeGamma fakeGamma,
                                       const SkMatrix& viewMatrix,
                                       const SkSurfaceProps& props,
                                       const char text[], size_t byteLength,
                                       SkScalar x, SkScalar y) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob = blobCache->createBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, x, y);

    if (GrTextUtils::CanDrawAsDistanceFields(skPaint, viewMatrix, props, shaderCaps)) {
        GrTextUtils::DrawDFText(blob, 0, fontCache, props,
                                skPaint, paint.getColor(), fakeGamma, viewMatrix, text,
                                byteLength, x, y);
    } else {
        GrTextUtils::DrawBmpText(blob, 0, fontCache, props, skPaint,
                                 paint.getColor(), fakeGamma, viewMatrix, text, byteLength, x, y);
    }
    return blob;
}

inline GrAtlasTextBlob*
GrAtlasTextContext::CreateDrawPosTextBlob(GrTextBlobCache* blobCache, GrBatchFontCache* fontCache,
                                          const GrShaderCaps& shaderCaps, const GrPaint& paint,
                                          const SkPaint& skPaint, SkPaint::FakeGamma fakeGamma,
                                          const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                          const char text[], size_t byteLength,
                                          const SkScalar pos[], int scalarsPerPosition,
                                          const SkPoint& offset) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob = blobCache->createBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, offset.x(), offset.y());

    if (GrTextUtils::CanDrawAsDistanceFields(skPaint, viewMatrix, props, shaderCaps)) {
        GrTextUtils::DrawDFPosText(blob, 0, fontCache, props,
                                   skPaint, paint.getColor(), fakeGamma, viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset);
    } else {
        GrTextUtils::DrawBmpPosText(blob, 0, fontCache, props, skPaint,
                                    paint.getColor(), fakeGamma, viewMatrix, text,
                                    byteLength, pos, scalarsPerPosition, offset);
    }
    return blob;
}

void GrAtlasTextContext::drawText(GrContext* context,
                                  GrDrawContext* dc,
                                  const GrClip& clip,
                                  const GrPaint& paint, const SkPaint& skPaint,
                                  const SkMatrix& viewMatrix,
                                  const SkSurfaceProps& props,
                                  const char text[], size_t byteLength,
                                  SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix, props, *context->caps()->shaderCaps())) {
        SkAutoTUnref<GrAtlasTextBlob> blob(
            CreateDrawTextBlob(context->getTextBlobCache(), context->getBatchFontCache(),
                               *context->caps()->shaderCaps(),
                               paint, skPaint,
                               ComputeFakeGamma(dc, paint),
                               viewMatrix, props,
                               text, byteLength, x, y));
        blob->flushThrowaway(context, dc, props, fDistanceAdjustTable, skPaint, paint,
                             clip, viewMatrix, regionClipBounds, x, y);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawTextAsPath(context, dc, clip, skPaint, viewMatrix, text, byteLength, x, y,
                                regionClipBounds);
}

void GrAtlasTextContext::drawPosText(GrContext* context,
                                     GrDrawContext* dc,
                                     const GrClip& clip,
                                     const GrPaint& paint, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix,
                                     const SkSurfaceProps& props,
                                     const char text[], size_t byteLength,
                                     const SkScalar pos[], int scalarsPerPosition,
                                     const SkPoint& offset, const SkIRect& regionClipBounds) {
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix, props, *context->caps()->shaderCaps())) {
        SkAutoTUnref<GrAtlasTextBlob> blob(
            CreateDrawPosTextBlob(context->getTextBlobCache(),
                                  context->getBatchFontCache(),
                                  *context->caps()->shaderCaps(),
                                  paint, skPaint,
                                  ComputeFakeGamma(dc, paint),
                                  viewMatrix, props,
                                  text, byteLength,
                                  pos, scalarsPerPosition,
                                  offset));
        blob->flushThrowaway(context, dc, props, fDistanceAdjustTable, skPaint, paint,
                             clip, viewMatrix, regionClipBounds, offset.fX, offset.fY);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawPosTextAsPath(context, dc, props, clip, skPaint, viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset, regionClipBounds);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(TextBlobBatch) {
    static uint32_t gContextID = SK_InvalidGenID;
    static GrAtlasTextContext* gTextContext = nullptr;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        delete gTextContext;

        gTextContext = GrAtlasTextContext::Create();
    }

    // Setup dummy SkPaint / GrPaint
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPaint skPaint;
    skPaint.setColor(color);
    skPaint.setLCDRenderText(random->nextBool());
    skPaint.setAntiAlias(skPaint.isLCDRenderText() ? true : random->nextBool());
    skPaint.setSubpixelText(random->nextBool());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, skPaint, viewMatrix, gSurfaceProps.allowSRGBInputs(),
                          &grPaint)) {
        SkFAIL("couldn't convert paint\n");
    }

    const char* text = "The quick brown fox jumps over the lazy dog.";
    int textLen = (int)strlen(text);

    // create some random x/y offsets, including negative offsets
    static const int kMaxTrans = 1024;
    int xPos = (random->nextU() % 2) * 2 - 1;
    int yPos = (random->nextU() % 2) * 2 - 1;
    int xInt = (random->nextU() % kMaxTrans) * xPos;
    int yInt = (random->nextU() % kMaxTrans) * yPos;
    SkScalar x = SkIntToScalar(xInt);
    SkScalar y = SkIntToScalar(yInt);

    // right now we don't handle textblobs, nor do we handle drawPosText.  Since we only
    // intend to test the batch with this unit test, that is okay.
    SkAutoTUnref<GrAtlasTextBlob> blob(
        GrAtlasTextContext::CreateDrawTextBlob(context->getTextBlobCache(),
                                               context->getBatchFontCache(),
                                               *context->caps()->shaderCaps(), grPaint, skPaint,
                                               GrAtlasTextContext::kTextBlobBatchFakeGamma,
                                               viewMatrix,
                                               gSurfaceProps, text,
                                               static_cast<size_t>(textLen), x, y));

    return blob->test_createBatch(textLen, 0, 0, viewMatrix, x, y, color, skPaint,
                                  gSurfaceProps, gTextContext->dfAdjustTable(),
                                  context->getBatchFontCache());
}

#endif
