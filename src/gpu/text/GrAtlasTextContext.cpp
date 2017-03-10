/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrTextBlobCache.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkGr.h"
#include "ops/GrMeshDrawOp.h"

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

SkColor GrAtlasTextContext::ComputeCanonicalColor(const SkPaint& paint, bool lcd) {
    SkColor canonicalColor = paint.computeLuminanceColor();
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

uint32_t GrAtlasTextContext::ComputeScalerContextFlags(GrRenderTargetContext* rtc) {
    // If we're doing gamma-correct rendering, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    if (rtc->isGammaCorrect()) {
        return SkPaint::kBoostContrast_ScalerContextFlag;
    } else {
        return SkPaint::kFakeGammaAndBoostContrast_ScalerContextFlags;
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

void GrAtlasTextContext::drawTextBlob(GrContext* context, GrRenderTargetContext* rtc,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      const SkMatrix& viewMatrix,
                                      const SkSurfaceProps& props, const SkTextBlob* blob,
                                      SkScalar x, SkScalar y,
                                      SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (context->abandoned()) {
        return;
    }

    sk_sp<GrAtlasTextBlob> cacheBlob;
    SkMaskFilter::BlurRec blurRec;
    GrAtlasTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() ||
                      (mf && !mf->asABlur(&blurRec)) ||
                      drawFilter);
    uint32_t scalerContextFlags = ComputeScalerContextFlags(rtc);

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
        key.fScalerContextFlags = scalerContextFlags;
        cacheBlob = cache->find(key);
    }

    GrTextUtils::Paint paint(&skPaint);
    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(paint, blurRec, viewMatrix, x, y)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            cache->remove(cacheBlob.get());
            cacheBlob = cache->makeCachedBlob(blob, key, blurRec, skPaint);
            RegenerateTextBlob(cacheBlob.get(), context->getAtlasGlyphCache(),
                               *context->caps()->shaderCaps(), paint, scalerContextFlags,
                               viewMatrix, props, blob, x, y, drawFilter);
        } else {
            cache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                sk_sp<GrAtlasTextBlob> sanityBlob(cache->makeBlob(glyphCount, runCount));
                sanityBlob->setupKey(key, blurRec, skPaint);
                RegenerateTextBlob(sanityBlob.get(), context->getAtlasGlyphCache(),
                                   *context->caps()->shaderCaps(), paint, scalerContextFlags,
                                   viewMatrix, props, blob, x, y, drawFilter);
                GrAtlasTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = cache->makeCachedBlob(blob, key, blurRec, skPaint);
        } else {
            cacheBlob = cache->makeBlob(blob);
        }
        RegenerateTextBlob(cacheBlob.get(), context->getAtlasGlyphCache(),
                           *context->caps()->shaderCaps(), paint, scalerContextFlags, viewMatrix,
                           props, blob, x, y, drawFilter);
    }

    cacheBlob->flushCached(context, rtc, blob, props, fDistanceAdjustTable.get(), paint, drawFilter,
                           clip, viewMatrix, clipBounds, x, y);
}

void GrAtlasTextContext::RegenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            GrAtlasGlyphCache* fontCache,
                                            const GrShaderCaps& shaderCaps,
                                            const GrTextUtils::Paint& paint,
                                            uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                                            const SkSurfaceProps& props, const SkTextBlob* blob,
                                            SkScalar x, SkScalar y, SkDrawFilter* drawFilter) {
    cacheBlob->initReusableBlob(paint.filteredSkColor(), viewMatrix, x, y);

    // Regenerate textblob
    SkTextBlobRunIterator it(blob);
    GrTextUtils::RunPaint runPaint(&paint, drawFilter, props);
    for (int run = 0; !it.done(); it.next(), run++) {
        int glyphCount = it.glyphCount();
        size_t textLen = glyphCount * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        cacheBlob->push_back_run(run);
        if (!runPaint.modifyForRun(it)) {
            continue;
        }
        if (GrTextUtils::CanDrawAsDistanceFields(runPaint, viewMatrix, props, shaderCaps)) {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    GrTextUtils::DrawDFText(cacheBlob, run, fontCache, props, runPaint,
                                            scalerContextFlags, viewMatrix,
                                            (const char*)it.glyphs(), textLen, x + offset.x(),
                                            y + offset.y());
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y + offset.y());
                    GrTextUtils::DrawDFPosText(
                            cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                            viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 1, dfOffset);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y);
                    GrTextUtils::DrawDFPosText(
                            cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                            viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 2, dfOffset);
                    break;
                }
            }
        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            cacheBlob->setRunDrawAsPaths(run);
        } else {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning:
                    GrTextUtils::DrawBmpText(cacheBlob, run, fontCache, props, runPaint,
                                             scalerContextFlags, viewMatrix,
                                             (const char*)it.glyphs(), textLen, x + offset.x(),
                                             y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fontCache, props, runPaint,
                                                scalerContextFlags, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 1,
                                                SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fontCache, props, runPaint,
                                                scalerContextFlags, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 2,
                                                SkPoint::Make(x, y));
                    break;
            }
        }
    }
}

inline sk_sp<GrAtlasTextBlob>
GrAtlasTextContext::MakeDrawTextBlob(GrTextBlobCache* blobCache,
                                     GrAtlasGlyphCache* fontCache,
                                     const GrShaderCaps& shaderCaps,
                                     const GrTextUtils::Paint& paint,
                                     uint32_t scalerContextFlags,
                                     const SkMatrix& viewMatrix,
                                     const SkSurfaceProps& props,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y) {
    int glyphCount = paint.skPaint().countText(text, byteLength);

    sk_sp<GrAtlasTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, x, y);

    if (GrTextUtils::CanDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps)) {
        GrTextUtils::DrawDFText(blob.get(), 0, fontCache, props, paint, scalerContextFlags,
                                viewMatrix, text, byteLength, x, y);
    } else {
        GrTextUtils::DrawBmpText(blob.get(), 0, fontCache, props, paint, scalerContextFlags,
                                 viewMatrix, text, byteLength, x, y);
    }
    return blob;
}

inline sk_sp<GrAtlasTextBlob>
GrAtlasTextContext::MakeDrawPosTextBlob(GrTextBlobCache* blobCache,
                                        GrAtlasGlyphCache* fontCache,
                                        const GrShaderCaps& shaderCaps,
                                        const GrTextUtils::Paint& paint,
                                        uint32_t scalerContextFlags,
                                        const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props,
                                        const char text[], size_t byteLength,
                                        const SkScalar pos[], int scalarsPerPosition, const
                                        SkPoint& offset) {
    int glyphCount = paint.skPaint().countText(text, byteLength);

    sk_sp<GrAtlasTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, offset.x(), offset.y());

    if (GrTextUtils::CanDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps)) {
        GrTextUtils::DrawDFPosText(blob.get(), 0, fontCache, props, paint, scalerContextFlags,
                                   viewMatrix, text, byteLength, pos, scalarsPerPosition, offset);
    } else {
        GrTextUtils::DrawBmpPosText(blob.get(), 0, fontCache, props, paint, scalerContextFlags,
                                    viewMatrix, text, byteLength, pos, scalarsPerPosition, offset);
    }
    return blob;
}

void GrAtlasTextContext::drawText(GrContext* context, GrRenderTargetContext* rtc,
                                  const GrClip& clip, const SkPaint& skPaint,
                                  const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                  const char text[], size_t byteLength, SkScalar x, SkScalar y,
                                  const SkIRect& regionClipBounds) {
    if (context->abandoned()) {
        return;
    }
    GrTextUtils::Paint paint(&skPaint);
    if (this->canDraw(skPaint, viewMatrix, props, *context->caps()->shaderCaps())) {
        sk_sp<GrAtlasTextBlob> blob(
            MakeDrawTextBlob(context->getTextBlobCache(), context->getAtlasGlyphCache(),
                             *context->caps()->shaderCaps(),
                             paint, ComputeScalerContextFlags(rtc),
                             viewMatrix, props,
                             text, byteLength, x, y));
        blob->flushThrowaway(context, rtc, props, fDistanceAdjustTable.get(), paint, clip,
                             viewMatrix, regionClipBounds, x, y);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawTextAsPath(context, rtc, clip, paint, viewMatrix, text, byteLength, x, y,
                                regionClipBounds);
}

void GrAtlasTextContext::drawPosText(GrContext* context, GrRenderTargetContext* rtc,
                                     const GrClip& clip, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                     const char text[], size_t byteLength, const SkScalar pos[],
                                     int scalarsPerPosition, const SkPoint& offset,
                                     const SkIRect& regionClipBounds) {
    GrTextUtils::Paint paint(&skPaint);
    if (context->abandoned()) {
        return;
    } else if (this->canDraw(skPaint, viewMatrix, props, *context->caps()->shaderCaps())) {
        sk_sp<GrAtlasTextBlob> blob(
            MakeDrawPosTextBlob(context->getTextBlobCache(), context->getAtlasGlyphCache(),
                                *context->caps()->shaderCaps(),
                                paint, ComputeScalerContextFlags(rtc),
                                viewMatrix, props,
                                text, byteLength,
                                pos, scalarsPerPosition,
                                offset));
        blob->flushThrowaway(context, rtc, props, fDistanceAdjustTable.get(), paint, clip,
                             viewMatrix, regionClipBounds, offset.fX, offset.fY);
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawPosTextAsPath(context, rtc, props, clip, paint, viewMatrix, text, byteLength,
                                   pos, scalarsPerPosition, offset, regionClipBounds);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

DRAW_OP_TEST_DEFINE(TextBlobOp) {
    static uint32_t gContextID = SK_InvalidGenID;
    static GrAtlasTextContext* gTextContext = nullptr;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        delete gTextContext;

        gTextContext = GrAtlasTextContext::Create();
    }

    // Setup dummy SkPaint / GrPaint / GrRenderTargetContext
    sk_sp<GrRenderTargetContext> renderTargetContext(context->makeRenderTargetContext(
        SkBackingFit::kApprox, 1024, 1024, kRGBA_8888_GrPixelConfig, nullptr));

    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPaint skPaint;
    skPaint.setColor(random->nextU());
    skPaint.setLCDRenderText(random->nextBool());
    skPaint.setAntiAlias(skPaint.isLCDRenderText() ? true : random->nextBool());
    skPaint.setSubpixelText(random->nextBool());

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

    GrTextUtils::Paint paint(&skPaint);
    // right now we don't handle textblobs, nor do we handle drawPosText. Since we only intend to
    // test the text op with this unit test, that is okay.
    sk_sp<GrAtlasTextBlob> blob(GrAtlasTextContext::MakeDrawTextBlob(
            context->getTextBlobCache(), context->getAtlasGlyphCache(),
            *context->caps()->shaderCaps(), paint,
            GrAtlasTextContext::kTextBlobOpScalerContextFlags, viewMatrix, gSurfaceProps, text,
            static_cast<size_t>(textLen), x, y));

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, paint, gSurfaceProps,
                             gTextContext->dfAdjustTable(), context->getAtlasGlyphCache());
}

#endif
