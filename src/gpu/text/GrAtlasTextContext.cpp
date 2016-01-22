/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"

#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrStrokeInfo.h"
#include "GrTextBlobCache.h"
#include "GrTexturePriv.h"
#include "GrTextUtils.h"
#include "GrVertexBuffer.h"

#include "SkAutoKern.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkGrPriv.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "SkTextBlob.h"
#include "SkTextMapStateProc.h"

#include "batches/GrAtlasTextBatch.h"

GrAtlasTextContext::GrAtlasTextContext(GrContext* context, const SkSurfaceProps& surfaceProps)
    : INHERITED(context, surfaceProps)
    , fDistanceAdjustTable(new GrDistanceFieldAdjustTable) {
    // We overallocate vertices in our textblobs based on the assumption that A8 has the greatest
    // vertexStride
    static_assert(GrAtlasTextBlob::kGrayTextVASize >= GrAtlasTextBlob::kColorTextVASize &&
                  GrAtlasTextBlob::kGrayTextVASize >= GrAtlasTextBlob::kLCDTextVASize,
                  "vertex_attribute_changed");
    fCurrStrike = nullptr;
    fCache = context->getTextBlobCache();
}


GrAtlasTextContext* GrAtlasTextContext::Create(GrContext* context,
                                               const SkSurfaceProps& surfaceProps) {
    return new GrAtlasTextContext(context, surfaceProps);
}

bool GrAtlasTextContext::canDraw(const SkPaint& skPaint, const SkMatrix& viewMatrix) {
    return GrTextUtils::CanDrawAsDistanceFields(skPaint, viewMatrix, fSurfaceProps,
                                                *fContext->caps()->shaderCaps()) ||
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

void GrAtlasTextContext::drawTextBlob(GrDrawContext* dc,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                      SkScalar x, SkScalar y,
                                      SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (fContext->abandoned()) {
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

    if (canCache) {
        bool hasLCD = HasLCD(blob);

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? fSurfaceProps.pixelGeometry() :
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
        cacheBlob.reset(SkSafeRef(fCache->find(key)));
    }

    SkScalar transX = 0.f;
    SkScalar transY = 0.f;

    // Though for the time being runs in the textblob can override the paint, they only touch font
    // info.
    GrPaint grPaint;
    if (!SkPaintToGrPaint(fContext, skPaint, viewMatrix, &grPaint)) {
        return;
    }

    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(&transX, &transY, skPaint, grPaint.getColor(), blurRec,
                                      viewMatrix, x, y)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            fCache->remove(cacheBlob);
            cacheBlob.reset(SkRef(fCache->createCachedBlob(blob, key, blurRec, skPaint,
                                                           GrAtlasTextBlob::kGrayTextVASize)));
            this->regenerateTextBlob(cacheBlob, skPaint, grPaint.getColor(), viewMatrix,
                                     blob, x, y, drawFilter);
        } else {
            fCache->makeMRU(cacheBlob);
#ifdef CACHE_SANITY_CHECK
            {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                SkAutoTUnref<GrAtlasTextBlob> sanityBlob(fCache->createBlob(glyphCount, runCount,
                                                                            kGrayTextVASize));
                GrTextBlobCache::SetupCacheBlobKey(sanityBlob, key, blurRec, skPaint);
                this->regenerateTextBlob(sanityBlob, skPaint, grPaint.getColor(), viewMatrix,
                                         blob, x, y, drawFilter);
                GrAtlasTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }

#endif
        }
    } else {
        if (canCache) {
            cacheBlob.reset(SkRef(fCache->createCachedBlob(blob, key, blurRec, skPaint,
                                                           GrAtlasTextBlob::kGrayTextVASize)));
        } else {
            cacheBlob.reset(fCache->createBlob(blob, GrAtlasTextBlob::kGrayTextVASize));
        }
        this->regenerateTextBlob(cacheBlob, skPaint, grPaint.getColor(), viewMatrix,
                                 blob, x, y, drawFilter);
    }

    cacheBlob->flushCached(fContext, dc, blob, fSurfaceProps, fDistanceAdjustTable, skPaint,
                           grPaint, drawFilter, clip, viewMatrix, clipBounds, x, y, transX, transY);
}

void GrAtlasTextContext::regenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            const SkPaint& skPaint, GrColor color,
                                            const SkMatrix& viewMatrix,
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

        runPaint.setFlags(FilterTextFlags(fSurfaceProps, runPaint));

        cacheBlob->push_back_run(run);

        if (GrTextUtils::CanDrawAsDistanceFields(runPaint, viewMatrix, fSurfaceProps,
                                                 *fContext->caps()->shaderCaps())) {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    GrTextUtils::DrawDFText(cacheBlob, run, fContext->getBatchFontCache(),
                                            fSurfaceProps, runPaint, color, viewMatrix,
                                            (const char *)it.glyphs(), textLen,
                                            x + offset.x(), y + offset.y());
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y + offset.y());
                    GrTextUtils::DrawDFPosText(cacheBlob, run, fContext->getBatchFontCache(),
                                               fSurfaceProps, runPaint, color, viewMatrix,
                                               (const char*)it.glyphs(), textLen, it.pos(),
                                               1, dfOffset);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y);
                    GrTextUtils::DrawDFPosText(cacheBlob, run,  fContext->getBatchFontCache(),
                                               fSurfaceProps, runPaint, color, viewMatrix,
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
                    GrTextUtils::DrawBmpText(cacheBlob, run, fContext->getBatchFontCache(),
                                             fSurfaceProps, runPaint, color, viewMatrix,
                                             (const char *)it.glyphs(), textLen,
                                             x + offset.x(), y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fContext->getBatchFontCache(),
                                                fSurfaceProps, runPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 1,
                                                SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fContext->getBatchFontCache(),
                                                fSurfaceProps, runPaint, color, viewMatrix,
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
GrAtlasTextContext::createDrawTextBlob(const GrPaint& paint, const SkPaint& skPaint,
                                       const SkMatrix& viewMatrix,
                                       const char text[], size_t byteLength,
                                       SkScalar x, SkScalar y) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBlob::kGrayTextVASize);
    blob->initThrowawayBlob(viewMatrix, x, y);

    if (GrTextUtils::CanDrawAsDistanceFields(skPaint, viewMatrix, fSurfaceProps,
                                             *fContext->caps()->shaderCaps())) {
        GrTextUtils::DrawDFText(blob, 0, fContext->getBatchFontCache(), fSurfaceProps,
                                skPaint, paint.getColor(), viewMatrix, text,
                                byteLength, x, y);
    } else {
        GrTextUtils::DrawBmpText(blob, 0, fContext->getBatchFontCache(), fSurfaceProps, skPaint,
                                 paint.getColor(), viewMatrix, text, byteLength, x, y);
    }
    return blob;
}

inline GrAtlasTextBlob*
GrAtlasTextContext::createDrawPosTextBlob(const GrPaint& paint, const SkPaint& skPaint,
                                          const SkMatrix& viewMatrix,
                                          const char text[], size_t byteLength,
                                          const SkScalar pos[], int scalarsPerPosition,
                                          const SkPoint& offset) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBlob::kGrayTextVASize);
    blob->initThrowawayBlob(viewMatrix, offset.x(), offset.y());

    if (GrTextUtils::CanDrawAsDistanceFields(skPaint, viewMatrix, fSurfaceProps,
                                             *fContext->caps()->shaderCaps())) {
        GrTextUtils::DrawDFPosText(blob, 0, fContext->getBatchFontCache(), fSurfaceProps,
                                   skPaint, paint.getColor(), viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset);
    } else {
        GrTextUtils::DrawBmpPosText(blob, 0, fContext->getBatchFontCache(), fSurfaceProps, skPaint,
                                    paint.getColor(), viewMatrix, text,
                                    byteLength, pos, scalarsPerPosition, offset);
    }
    return blob;
}

void GrAtlasTextContext::onDrawText(GrDrawContext* dc,
                                    const GrClip& clip,
                                    const GrPaint& paint, const SkPaint& skPaint,
                                    const SkMatrix& viewMatrix,
                                    const char text[], size_t byteLength,
                                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    SkAutoTUnref<GrAtlasTextBlob> blob(
        this->createDrawTextBlob(paint, skPaint, viewMatrix, text, byteLength, x, y));
    blob->flushThrowaway(fContext, dc, fSurfaceProps, fDistanceAdjustTable, skPaint, paint,
                         clip, regionClipBounds);
}

void GrAtlasTextContext::onDrawPosText(GrDrawContext* dc,
                                       const GrClip& clip,
                                       const GrPaint& paint, const SkPaint& skPaint,
                                       const SkMatrix& viewMatrix,
                                       const char text[], size_t byteLength,
                                       const SkScalar pos[], int scalarsPerPosition,
                                       const SkPoint& offset, const SkIRect& regionClipBounds) {
    SkAutoTUnref<GrAtlasTextBlob> blob(
        this->createDrawPosTextBlob(paint, skPaint, viewMatrix,
                                    text, byteLength,
                                    pos, scalarsPerPosition,
                                    offset));

    blob->flushThrowaway(fContext, dc, fSurfaceProps, fDistanceAdjustTable, skPaint, paint, clip,
                         regionClipBounds);
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

        // We don't yet test the fall back to paths in the GrTextContext base class.  This is mostly
        // because we don't really want to have a gpu device here.
        // We enable distance fields by twiddling a knob on the paint
        gTextContext = GrAtlasTextContext::Create(context, gSurfaceProps);
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
    if (!SkPaintToGrPaint(context, skPaint, viewMatrix, &grPaint)) {
        SkFAIL("couldn't convert paint\n");
    }

    const char* text = "The quick brown fox jumps over the lazy dog.";
    int textLen = (int)strlen(text);

    // Setup clip
    GrClip clip;

    // right now we don't handle textblobs, nor do we handle drawPosText.  Since we only
    // intend to test the batch with this unit test, that is okay.
    SkAutoTUnref<GrAtlasTextBlob> blob(
            gTextContext->createDrawTextBlob(grPaint, skPaint, viewMatrix, text,
                                             static_cast<size_t>(textLen), 0, 0));

    // We'd like to be able to test this with random translations, but currently the vertex
    // bounds and vertices will get out of sync
    SkScalar transX = 0.f;//SkIntToScalar(random->nextU());
    SkScalar transY = 0.f;//SkIntToScalar(random->nextU());
    return blob->test_createBatch(textLen, 0, 0, color, transX, transY, skPaint,
                                  gSurfaceProps, gTextContext->dfAdjustTable(),
                                  context->getBatchFontCache());
}

#endif
