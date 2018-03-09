/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrTextBlobCache.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGr.h"
#include "SkGraphics.h"
#include "SkMakeUnique.h"
#include "SkMaskFilterBase.h"
#include "SkTextMapStateProc.h"

#include "ops/GrMeshDrawOp.h"

// DF sizes and thresholds for usage of the small and medium sizes. For example, above
// kSmallDFFontLimit we will use the medium size. The large size is used up until the size at
// which we switch over to drawing as paths as controlled by Options.
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;

static const int kDefaultMinDistanceFieldFontSize = 18;
#ifdef SK_BUILD_FOR_ANDROID
static const int kDefaultMaxDistanceFieldFontSize = 384;
#else
static const int kDefaultMaxDistanceFieldFontSize = 2 * kLargeDFFontSize;
#endif

GrAtlasTextContext::GrAtlasTextContext(const Options& options)
        : fDistanceAdjustTable(new GrDistanceFieldAdjustTable) {
    fMaxDistanceFieldFontSize = options.fMaxDistanceFieldFontSize < 0.f
                                        ? kDefaultMaxDistanceFieldFontSize
                                        : options.fMaxDistanceFieldFontSize;
    fMinDistanceFieldFontSize = options.fMinDistanceFieldFontSize < 0.f
                                        ? kDefaultMinDistanceFieldFontSize
                                        : options.fMinDistanceFieldFontSize;
    fDistanceFieldVerticesAlwaysHaveW = options.fDistanceFieldVerticesAlwaysHaveW;
}

std::unique_ptr<GrAtlasTextContext> GrAtlasTextContext::Make(const Options& options) {
    return std::unique_ptr<GrAtlasTextContext>(new GrAtlasTextContext(options));
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

SkScalerContextFlags GrAtlasTextContext::ComputeScalerContextFlags(
        const GrColorSpaceInfo& colorSpaceInfo) {
    // If we're doing gamma-correct rendering, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    if (colorSpaceInfo.isGammaCorrect()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
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

void GrAtlasTextContext::drawTextBlob(GrContext* context, GrTextUtils::Target* target,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                      const SkTextBlob* blob, SkScalar x, SkScalar y,
                                      SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (context->abandoned()) {
        return;
    }

    sk_sp<GrAtlasTextBlob> cacheBlob;
    SkMaskFilterBase::BlurRec blurRec;
    GrAtlasTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() ||
                      (mf && !as_MFB(mf)->asABlur(&blurRec)) ||
                      drawFilter);
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(target->colorSpaceInfo());

    auto glyphCache = context->contextPriv().getGlyphCache();
    auto restrictedAtlasManager = context->contextPriv().getRestrictedAtlasManager();
    GrTextBlobCache* textBlobCache = context->contextPriv().getTextBlobCache();

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
        cacheBlob = textBlobCache->find(key);
    }

    GrTextUtils::Paint paint(&skPaint, &target->colorSpaceInfo());
    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(paint, blurRec, viewMatrix, x, y)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cacheBlob.get());
            cacheBlob = textBlobCache->makeCachedBlob(blob, key, blurRec, skPaint);
            this->regenerateTextBlob(cacheBlob.get(), glyphCache,
                                     *context->caps()->shaderCaps(), paint, scalerContextFlags,
                                     viewMatrix, props, blob, x, y, drawFilter);
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                sk_sp<GrAtlasTextBlob> sanityBlob(textBlobCache->makeBlob(glyphCount, runCount));
                sanityBlob->setupKey(key, blurRec, skPaint);
                this->regenerateTextBlob(sanityBlob.get(), glyphCache,
                                         *context->caps()->shaderCaps(), paint, scalerContextFlags,
                                         viewMatrix, props, blob, x, y, drawFilter);
                GrAtlasTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(blob, key, blurRec, skPaint);
        } else {
            cacheBlob = textBlobCache->makeBlob(blob);
        }
        this->regenerateTextBlob(cacheBlob.get(), glyphCache,
                                 *context->caps()->shaderCaps(), paint, scalerContextFlags,
                                 viewMatrix, props, blob, x, y, drawFilter);
    }

    cacheBlob->flush(restrictedAtlasManager, target, props, fDistanceAdjustTable.get(), paint,
                     clip, viewMatrix, clipBounds, x, y);
}

void GrAtlasTextContext::regenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            GrGlyphCache* glyphCache,
                                            const GrShaderCaps& shaderCaps,
                                            const GrTextUtils::Paint& paint,
                                            SkScalerContextFlags scalerContextFlags,
                                            const SkMatrix& viewMatrix,
                                            const SkSurfaceProps& props, const SkTextBlob* blob,
                                            SkScalar x, SkScalar y,
                                            SkDrawFilter* drawFilter) const {
    cacheBlob->initReusableBlob(paint.luminanceColor(), viewMatrix, x, y);

    // Regenerate textblob
    SkTextBlobRunIterator it(blob);
    GrTextUtils::RunPaint runPaint(&paint, drawFilter, props);
    for (int run = 0; !it.done(); it.next(), run++) {
        int glyphCount = it.glyphCount();
        size_t textLen = glyphCount * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        cacheBlob->push_back_run(run);
        if (!runPaint.modifyForRun([it](SkPaint* p) { it.applyFontToPaint(p); })) {
            continue;
        }
        cacheBlob->setRunPaintFlags(run, runPaint.skPaint().getFlags());

        if (this->canDrawAsDistanceFields(runPaint, viewMatrix, props, shaderCaps)) {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    this->drawDFText(cacheBlob, run, glyphCache, props, runPaint, scalerContextFlags,
                                     viewMatrix, (const char*)it.glyphs(), textLen, x + offset.x(),
                                     y + offset.y());
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y + offset.y());
                    this->drawDFPosText(cacheBlob, run, glyphCache, props, runPaint,
                                        scalerContextFlags, viewMatrix, (const char*)it.glyphs(),
                                        textLen, it.pos(), 1, dfOffset);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y);
                    this->drawDFPosText(cacheBlob, run, glyphCache, props, runPaint,
                                        scalerContextFlags, viewMatrix, (const char*)it.glyphs(),
                                        textLen, it.pos(), 2, dfOffset);
                    break;
                }
            }
        } else {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning:
                    DrawBmpText(cacheBlob, run, glyphCache, props, runPaint, scalerContextFlags,
                                viewMatrix, (const char*)it.glyphs(), textLen, x + offset.x(),
                                y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    DrawBmpPosText(cacheBlob, run, glyphCache, props, runPaint, scalerContextFlags,
                                   viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 1,
                                   SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    DrawBmpPosText(cacheBlob, run, glyphCache, props, runPaint, scalerContextFlags,
                                   viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 2,
                                   SkPoint::Make(x, y));
                    break;
            }
        }
    }
}

inline sk_sp<GrAtlasTextBlob>
GrAtlasTextContext::makeDrawTextBlob(GrTextBlobCache* blobCache,
                                     GrGlyphCache* glyphCache,
                                     const GrShaderCaps& shaderCaps,
                                     const GrTextUtils::Paint& paint,
                                     SkScalerContextFlags scalerContextFlags,
                                     const SkMatrix& viewMatrix,
                                     const SkSurfaceProps& props,
                                     const char text[], size_t byteLength,
                                     SkScalar x, SkScalar y) const {
    int glyphCount = paint.skPaint().countText(text, byteLength);
    if (!glyphCount) {
        return nullptr;
    }
    sk_sp<GrAtlasTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, x, y);
    blob->setRunPaintFlags(0, paint.skPaint().getFlags());

    if (this->canDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps)) {
        this->drawDFText(blob.get(), 0, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                         text, byteLength, x, y);
    } else {
        DrawBmpText(blob.get(), 0, glyphCache, props, paint, scalerContextFlags, viewMatrix, text,
                    byteLength, x, y);
    }
    return blob;
}

inline sk_sp<GrAtlasTextBlob>
GrAtlasTextContext::makeDrawPosTextBlob(GrTextBlobCache* blobCache,
                                        GrGlyphCache* glyphCache,
                                        const GrShaderCaps& shaderCaps,
                                        const GrTextUtils::Paint& paint,
                                        SkScalerContextFlags scalerContextFlags,
                                        const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props,
                                        const char text[], size_t byteLength,
                                        const SkScalar pos[], int scalarsPerPosition, const
                                        SkPoint& offset) const {
    int glyphCount = paint.skPaint().countText(text, byteLength);
    if (!glyphCount) {
        return nullptr;
    }

    sk_sp<GrAtlasTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, offset.x(), offset.y());
    blob->setRunPaintFlags(0, paint.skPaint().getFlags());

    if (this->canDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps)) {
        this->drawDFPosText(blob.get(), 0, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                            text, byteLength, pos, scalarsPerPosition, offset);
    } else {
        DrawBmpPosText(blob.get(), 0, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                       text, byteLength, pos, scalarsPerPosition, offset);
    }
    return blob;
}

void GrAtlasTextContext::drawText(GrContext* context, GrTextUtils::Target* target,
                                  const GrClip& clip, const SkPaint& skPaint,
                                  const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                  const char text[], size_t byteLength, SkScalar x, SkScalar y,
                                  const SkIRect& regionClipBounds) {
    if (context->abandoned()) {
        return;
    }

    auto glyphCache = context->contextPriv().getGlyphCache();
    auto restrictedAtlasManager = context->contextPriv().getRestrictedAtlasManager();
    auto textBlobCache = context->contextPriv().getTextBlobCache();

    GrTextUtils::Paint paint(&skPaint, &target->colorSpaceInfo());
    sk_sp<GrAtlasTextBlob> blob(
            this->makeDrawTextBlob(textBlobCache, glyphCache,
                                    *context->caps()->shaderCaps(), paint,
                                    ComputeScalerContextFlags(target->colorSpaceInfo()),
                                    viewMatrix, props, text, byteLength, x, y));
    if (blob) {
        blob->flush(restrictedAtlasManager, target, props, fDistanceAdjustTable.get(), paint,
                    clip, viewMatrix, regionClipBounds, x, y);
    }
}

void GrAtlasTextContext::drawPosText(GrContext* context, GrTextUtils::Target* target,
                                     const GrClip& clip, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                     const char text[], size_t byteLength, const SkScalar pos[],
                                     int scalarsPerPosition, const SkPoint& offset,
                                     const SkIRect& regionClipBounds) {
    GrTextUtils::Paint paint(&skPaint, &target->colorSpaceInfo());
    if (context->abandoned()) {
        return;
    }

    auto glyphCache = context->contextPriv().getGlyphCache();
    auto restrictedAtlasManager = context->contextPriv().getRestrictedAtlasManager();
    auto textBlobCache = context->contextPriv().getTextBlobCache();

    sk_sp<GrAtlasTextBlob> blob(this->makeDrawPosTextBlob(
            textBlobCache, glyphCache,
            *context->caps()->shaderCaps(), paint,
            ComputeScalerContextFlags(target->colorSpaceInfo()), viewMatrix, props, text,
            byteLength, pos, scalarsPerPosition, offset));
    if (blob) {
        blob->flush(restrictedAtlasManager, target, props, fDistanceAdjustTable.get(), paint,
                    clip, viewMatrix, regionClipBounds, offset.fX, offset.fY);
    }
}

void GrAtlasTextContext::DrawBmpText(GrAtlasTextBlob* blob, int runIndex,
                                     GrGlyphCache* glyphCache, const SkSurfaceProps& props,
                                     const GrTextUtils::Paint& paint,
                                     SkScalerContextFlags scalerContextFlags,
                                     const SkMatrix& viewMatrix, const char text[],
                                     size_t byteLength, SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // Ensure the blob is set for bitmaptext
    blob->setHasBitmap();

    if (SkDraw::ShouldDrawTextAsPaths(paint, viewMatrix)) {
        DrawBmpTextAsPaths(blob, runIndex, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                           text, byteLength, x, y);
        return;
    }

    sk_sp<GrTextStrike> currStrike;
    SkGlyphCache* cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);
    SkFindAndPlaceGlyph::ProcessText(paint.skPaint().getTextEncoding(), text, byteLength, {x, y},
                                     viewMatrix, paint.skPaint().getTextAlign(), cache,
                                     [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
                                         position += rounding;
                                         BmpAppendGlyph(blob, runIndex, glyphCache, &currStrike,
                                                        glyph, SkScalarFloorToScalar(position.fX),
                                                        SkScalarFloorToScalar(position.fY),
                                                        paint.filteredPremulColor(), cache,
                                                        SK_Scalar1);
                                     });

    SkGlyphCache::AttachCache(cache);
}

void GrAtlasTextContext::DrawBmpPosText(GrAtlasTextBlob* blob, int runIndex,
                                        GrGlyphCache* glyphCache, const SkSurfaceProps& props,
                                        const GrTextUtils::Paint& paint,
                                        SkScalerContextFlags scalerContextFlags,
                                        const SkMatrix& viewMatrix,
                                        const char text[], size_t byteLength, const SkScalar pos[],
                                        int scalarsPerPosition, const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // Ensure the blob is set for bitmaptext
    blob->setHasBitmap();

    if (SkDraw::ShouldDrawTextAsPaths(paint, viewMatrix)) {
        DrawBmpPosTextAsPaths(blob, runIndex, glyphCache, props, paint, scalerContextFlags,
                              viewMatrix, text, byteLength, pos, scalarsPerPosition, offset);
        return;
    }

    sk_sp<GrTextStrike> currStrike;
    SkGlyphCache* cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);
    SkFindAndPlaceGlyph::ProcessPosText(
            paint.skPaint().getTextEncoding(), text, byteLength, offset, viewMatrix, pos,
            scalarsPerPosition, paint.skPaint().getTextAlign(), cache,
            [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
                position += rounding;
                BmpAppendGlyph(blob, runIndex, glyphCache, &currStrike, glyph,
                               SkScalarFloorToScalar(position.fX),
                               SkScalarFloorToScalar(position.fY),
                               paint.filteredPremulColor(), cache, SK_Scalar1);
            });

    SkGlyphCache::AttachCache(cache);
}

void GrAtlasTextContext::DrawBmpTextAsPaths(GrAtlasTextBlob* blob, int runIndex,
                                            GrGlyphCache* glyphCache,
                                            const SkSurfaceProps& props,
                                            const GrTextUtils::Paint& origPaint,
                                            SkScalerContextFlags scalerContextFlags,
                                            const SkMatrix& viewMatrix, const char text[],
                                            size_t byteLength, SkScalar x, SkScalar y) {
    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    SkPaint pathPaint(origPaint);
    pathPaint.setStyle(SkPaint::kFill_Style);
    pathPaint.setPathEffect(nullptr);

    GrTextUtils::PathTextIter iter(text, byteLength, pathPaint, true);
    FallbackTextHelper fallbackTextHelper(viewMatrix, pathPaint, glyphCache, iter.getPathScale());

    const SkGlyph* iterGlyph;
    const SkPath* iterPath;
    SkScalar xpos = 0;
    const char* lastText = text;
    while (iter.next(&iterGlyph, &iterPath, &xpos)) {
        if (iterGlyph) {
            SkPoint pos = SkPoint::Make(xpos + x, y);
            fallbackTextHelper.appendText(*iterGlyph, iter.getText() - lastText, lastText, pos);
        } else if (iterPath) {
            blob->appendPathGlyph(runIndex, *iterPath, xpos + x, y, iter.getPathScale(), false);
        }
        lastText = iter.getText();
    }

    fallbackTextHelper.drawText(blob, runIndex, glyphCache, props, origPaint, scalerContextFlags);
}

void GrAtlasTextContext::DrawBmpPosTextAsPaths(GrAtlasTextBlob* blob, int runIndex,
                                               GrGlyphCache* glyphCache,
                                               const SkSurfaceProps& props,
                                               const GrTextUtils::Paint& origPaint,
                                               SkScalerContextFlags scalerContextFlags,
                                               const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               const SkScalar pos[], int scalarsPerPosition,
                                               const SkPoint& offset) {
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // setup our std paint, in hopes of getting hits in the cache
    SkPaint pathPaint(origPaint);
    SkScalar matrixScale = pathPaint.setupForAsPaths();
    FallbackTextHelper fallbackTextHelper(viewMatrix, origPaint, glyphCache, matrixScale);

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    pathPaint.setStyle(SkPaint::kFill_Style);
    pathPaint.setPathEffect(nullptr);

    SkPaint::GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(pathPaint.getTextEncoding(),
                                                                        pathPaint.isDevKernText(),
                                                                        true);
    SkAutoGlyphCache           autoCache(pathPaint, &props, nullptr);
    SkGlyphCache*              cache = autoCache.getCache();

    const char*        stop = text + byteLength;
    const char*        lastText = text;
    SkTextAlignProc    alignProc(pathPaint.getTextAlign());
    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text);
        if (glyph.fWidth) {
            SkPoint tmsLoc;
            tmsProc(pos, &tmsLoc);
            SkPoint loc;
            alignProc(tmsLoc, glyph, &loc);
            if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
                fallbackTextHelper.appendText(glyph, text - lastText, lastText, loc);
            } else {
                const SkPath* path = cache->findPath(glyph);
                if (path) {
                    blob->appendPathGlyph(runIndex, *path, loc.fX, loc.fY, matrixScale, false);
                }
            }
        }
        lastText = text;
        pos += scalarsPerPosition;
    }

    fallbackTextHelper.drawText(blob, runIndex, glyphCache, props, origPaint, scalerContextFlags);
}

void GrAtlasTextContext::BmpAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                        GrGlyphCache* grGlyphCache,
                                        sk_sp<GrTextStrike>* strike,
                                        const SkGlyph& skGlyph, SkScalar sx, SkScalar sy,
                                        GrColor color, SkGlyphCache* skGlyphCache,
                                        SkScalar textRatio) {
    if (!*strike) {
        *strike = grGlyphCache->getStrike(skGlyphCache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kCoverage_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, skGlyphCache);
    if (!glyph) {
        return;
    }

    SkASSERT(skGlyph.fWidth == glyph->width());
    SkASSERT(skGlyph.fHeight == glyph->height());

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop);
    SkScalar width = SkIntToScalar(glyph->fBounds.width());
    SkScalar height = SkIntToScalar(glyph->fBounds.height());

    dx *= textRatio;
    dy *= textRatio;
    width *= textRatio;
    height *= textRatio;

    SkRect glyphRect = SkRect::MakeXYWH(sx + dx, sy + dy, width, height);

    blob->appendGlyph(runIndex, glyphRect, color, *strike, glyph, skGlyphCache, skGlyph, sx, sy,
                      textRatio, true);
}

bool GrAtlasTextContext::canDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                                 const SkSurfaceProps& props,
                                                 const GrShaderCaps& caps) const {
    if (!viewMatrix.hasPerspective()) {
        SkScalar maxScale = viewMatrix.getMaxScale();
        SkScalar scaledTextSize = maxScale * skPaint.getTextSize();
        // Hinted text looks far better at small resolutions
        // Scaling up beyond 2x yields undesireable artifacts
        if (scaledTextSize < fMinDistanceFieldFontSize ||
            scaledTextSize > fMaxDistanceFieldFontSize) {
            return false;
        }

        bool useDFT = props.isUseDeviceIndependentFonts();
#if SK_FORCE_DISTANCE_FIELD_TEXT
        useDFT = true;
#endif

        if (!useDFT && scaledTextSize < kLargeDFFontSize) {
            return false;
        }
    }

    // mask filters modify alpha, which doesn't translate well to distance
    if (skPaint.getMaskFilter() || !caps.shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

void GrAtlasTextContext::initDistanceFieldPaint(GrAtlasTextBlob* blob,
                                                SkPaint* skPaint,
                                                SkScalar* textRatio,
                                                const SkMatrix& viewMatrix) const {
    SkScalar textSize = skPaint->getTextSize();
    SkScalar scaledTextSize = textSize;

    if (viewMatrix.hasPerspective()) {
        // for perspective, we simply force to the medium size
        // TODO: compute a size based on approximate screen area
        scaledTextSize = kMediumDFFontLimit;
    } else {
        SkScalar maxScale = viewMatrix.getMaxScale();
        // if we have non-unity scale, we need to choose our base text size
        // based on the SkPaint's text size multiplied by the max scale factor
        // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
        if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
            scaledTextSize *= maxScale;
        }
    }

    // We have three sizes of distance field text, and within each size 'bucket' there is a floor
    // and ceiling.  A scale outside of this range would require regenerating the distance fields
    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = fMinDistanceFieldFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
        *textRatio = textSize / kSmallDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kSmallDFFontSize));
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
        *textRatio = textSize / kMediumDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kMediumDFFontSize));
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = fMaxDistanceFieldFontSize;
        *textRatio = textSize / kLargeDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kLargeDFFontSize));
    }

    // Because there can be multiple runs in the blob, we want the overall maxMinScale, and
    // minMaxScale to make regeneration decisions.  Specifically, we want the maximum minimum scale
    // we can tolerate before we'd drop to a lower mip size, and the minimum maximum scale we can
    // tolerate before we'd have to move to a large mip size.  When we actually test these values
    // we look at the delta in scale between the new viewmatrix and the old viewmatrix, and test
    // against these values to decide if we can reuse or not(ie, will a given scale change our mip
    // level)
    SkASSERT(dfMaskScaleFloor <= scaledTextSize && scaledTextSize <= dfMaskScaleCeil);
    blob->setMinAndMaxScale(dfMaskScaleFloor / scaledTextSize, dfMaskScaleCeil / scaledTextSize);

    skPaint->setAntiAlias(true);
    skPaint->setLCDRenderText(false);
    skPaint->setAutohinted(false);
    skPaint->setHinting(SkPaint::kNormal_Hinting);
    skPaint->setSubpixelText(true);
}

void GrAtlasTextContext::drawDFText(GrAtlasTextBlob* blob, int runIndex,
                                    GrGlyphCache* glyphCache, const SkSurfaceProps& props,
                                    const GrTextUtils::Paint& paint,
                                    SkScalerContextFlags scalerContextFlags,
                                    const SkMatrix& viewMatrix, const char text[],
                                    size_t byteLength, SkScalar x, SkScalar y) const {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    const SkPaint& skPaint = paint.skPaint();
    SkPaint::GlyphCacheProc glyphCacheProc =
            SkPaint::GetGlyphCacheProc(skPaint.getTextEncoding(), skPaint.isDevKernText(), true);
    SkAutoDescriptor desc;
    SkScalerContextEffects effects;
    // We apply the fake-gamma by altering the distance in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
        skPaint, &props, SkScalerContextFlags::kNone, nullptr, &desc, &effects);
    SkGlyphCache* origPaintCache =
            SkGlyphCache::DetachCache(skPaint.getTypeface(), effects, desc.getDesc());

    SkTArray<SkScalar> positions;

    const char* textPtr = text;
    SkScalar stopX = 0;
    SkScalar stopY = 0;
    SkScalar origin = 0;
    switch (skPaint.getTextAlign()) {
        case SkPaint::kRight_Align: origin = SK_Scalar1; break;
        case SkPaint::kCenter_Align: origin = SK_ScalarHalf; break;
        case SkPaint::kLeft_Align: origin = 0; break;
    }

    SkAutoKern autokern;
    const char* stop = text + byteLength;
    while (textPtr < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(origPaintCache, &textPtr);

        SkScalar width = SkFloatToScalar(glyph.fAdvanceX) + autokern.adjust(glyph);
        positions.push_back(stopX + origin * width);

        SkScalar height = SkFloatToScalar(glyph.fAdvanceY);
        positions.push_back(stopY + origin * height);

        stopX += width;
        stopY += height;
    }
    SkASSERT(textPtr == stop);

    SkGlyphCache::AttachCache(origPaintCache);

    // now adjust starting point depending on alignment
    SkScalar alignX = stopX;
    SkScalar alignY = stopY;
    if (skPaint.getTextAlign() == SkPaint::kCenter_Align) {
        alignX = SkScalarHalf(alignX);
        alignY = SkScalarHalf(alignY);
    } else if (skPaint.getTextAlign() == SkPaint::kLeft_Align) {
        alignX = 0;
        alignY = 0;
    }
    x -= alignX;
    y -= alignY;
    SkPoint offset = SkPoint::Make(x, y);

    this->drawDFPosText(blob, runIndex, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                        text, byteLength, positions.begin(), 2, offset);
}

void GrAtlasTextContext::drawDFPosText(GrAtlasTextBlob* blob, int runIndex,
                                       GrGlyphCache* glyphCache, const SkSurfaceProps& props,
                                       const GrTextUtils::Paint& paint,
                                       SkScalerContextFlags scalerContextFlags,
                                       const SkMatrix& viewMatrix, const char text[],
                                       size_t byteLength, const SkScalar pos[],
                                       int scalarsPerPosition, const SkPoint& offset) const {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    bool hasWCoord = viewMatrix.hasPerspective() || fDistanceFieldVerticesAlwaysHaveW;

    // Setup distance field paint and text ratio
    SkScalar textRatio;
    SkPaint dfPaint(paint);
    this->initDistanceFieldPaint(blob, &dfPaint, &textRatio, viewMatrix);
    blob->setHasDistanceField();
    blob->setSubRunHasDistanceFields(runIndex, paint.skPaint().isLCDRenderText(),
                                     paint.skPaint().isAntiAlias(), hasWCoord);

    FallbackTextHelper fallbackTextHelper(viewMatrix, paint, glyphCache, textRatio);

    sk_sp<GrTextStrike> currStrike;

    // We apply the fake-gamma by altering the distance in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkGlyphCache* cache =
            blob->setupCache(runIndex, props, SkScalerContextFlags::kNone, dfPaint, nullptr);
    SkPaint::GlyphCacheProc glyphCacheProc =
            SkPaint::GetGlyphCacheProc(dfPaint.getTextEncoding(), dfPaint.isDevKernText(), true);

    const char* stop = text + byteLength;

    SkPaint::Align align = dfPaint.getTextAlign();
    SkScalar alignMul = SkPaint::kCenter_Align == align ? SK_ScalarHalf :
                        (SkPaint::kRight_Align == align ? SK_Scalar1 : 0);
    while (text < stop) {
        const char* lastText = text;
        // the last 2 parameters are ignored
        const SkGlyph& glyph = glyphCacheProc(cache, &text);

        if (glyph.fWidth) {
            SkPoint glyphPos(offset);
            glyphPos.fX += pos[0] - SkFloatToScalar(glyph.fAdvanceX) * alignMul * textRatio;
            glyphPos.fY += (2 == scalarsPerPosition ? pos[1] : 0) -
                           SkFloatToScalar(glyph.fAdvanceY) * alignMul * textRatio;

            if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
                DfAppendGlyph(blob, runIndex, glyphCache, &currStrike, glyph, glyphPos.fX,
                              glyphPos.fY, paint.filteredPremulColor(), cache, textRatio);
            } else {
                // can't append color glyph to SDF batch, send to fallback
                fallbackTextHelper.appendText(glyph, SkToInt(text - lastText), lastText, glyphPos);
            }
        }
        pos += scalarsPerPosition;
    }

    SkGlyphCache::AttachCache(cache);

    fallbackTextHelper.drawText(blob, runIndex, glyphCache, props, paint, scalerContextFlags);
}

// TODO: merge with BmpAppendGlyph
void GrAtlasTextContext::DfAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                       GrGlyphCache* grGlyphCache, sk_sp<GrTextStrike>* strike,
                                       const SkGlyph& skGlyph, SkScalar sx, SkScalar sy,
                                       GrColor color, SkGlyphCache* skGlyphCache,
                                       SkScalar textRatio) {
    if (!*strike) {
        *strike = grGlyphCache->getStrike(skGlyphCache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kDistance_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, skGlyphCache);
    if (!glyph) {
        return;
    }

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft + SK_DistanceFieldInset);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop + SK_DistanceFieldInset);
    SkScalar width = SkIntToScalar(glyph->fBounds.width() - 2 * SK_DistanceFieldInset);
    SkScalar height = SkIntToScalar(glyph->fBounds.height() - 2 * SK_DistanceFieldInset);

    dx *= textRatio;
    dy *= textRatio;
    width *= textRatio;
    height *= textRatio;
    SkRect glyphRect = SkRect::MakeXYWH(sx + dx, sy + dy, width, height);

    blob->appendGlyph(runIndex, glyphRect, color, *strike, glyph, skGlyphCache, skGlyph, sx, sy,
                      textRatio, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void GrAtlasTextContext::FallbackTextHelper::appendText(const SkGlyph& glyph, int count,
                                                        const char* text, SkPoint glyphPos) {
    SkScalar maxDim = SkTMax(glyph.fWidth, glyph.fHeight)*fTextRatio;
    if (!fUseScaledFallback) {
        SkScalar scaledGlyphSize = maxDim * fMaxScale;
        if (!fViewMatrix.hasPerspective() && scaledGlyphSize > fMaxTextSize) {
            fUseScaledFallback = true;
        }
    }

    fFallbackTxt.append(count, text);
    if (fUseScaledFallback) {
        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others pixelated, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar glyphTextSize = SkTMax(SkScalarFloorToScalar(fMaxTextSize*fTextSize / maxDim),
                                        0.5f*fMaxTextSize);
        fScaledFallbackTextSize = SkTMin(glyphTextSize, fScaledFallbackTextSize);
    }
    *fFallbackPos.append() = glyphPos;
}

void GrAtlasTextContext::FallbackTextHelper::drawText(GrAtlasTextBlob* blob, int runIndex,
                                                      GrGlyphCache* glyphCache,
                                                      const SkSurfaceProps& props,
                                                      const GrTextUtils::Paint& paint,
                                                      SkScalerContextFlags scalerContextFlags) {
    if (fFallbackTxt.count()) {
        blob->initOverride(runIndex);
        blob->setHasBitmap();
        SkGlyphCache* cache = nullptr;
        const SkPaint& skPaint = paint.skPaint();
        SkPaint::GlyphCacheProc glyphCacheProc =
            SkPaint::GetGlyphCacheProc(skPaint.getTextEncoding(),
                                       skPaint.isDevKernText(), true);
        SkColor textColor = paint.filteredPremulColor();
        SkScalar textRatio = SK_Scalar1;
        fViewMatrix.mapPoints(fFallbackPos.begin(), fFallbackPos.count());
        if (fUseScaledFallback) {
            // Set up paint and matrix to scale glyphs
            SkPaint scaledPaint(skPaint);
            scaledPaint.setTextSize(fScaledFallbackTextSize);
            // remove maxScale from viewMatrix and move it into textRatio
            // this keeps the base glyph size consistent regardless of matrix scale
            SkMatrix modMatrix(fViewMatrix);
            SkScalar invScale = SkScalarInvert(fMaxScale);
            modMatrix.preScale(invScale, invScale);
            textRatio = fTextSize * fMaxScale / fScaledFallbackTextSize;
            cache = blob->setupCache(runIndex, props, scalerContextFlags, scaledPaint,
                                     &modMatrix);
        } else {
            cache = blob->setupCache(runIndex, props, scalerContextFlags, paint,
                                     &fViewMatrix);
        }

        sk_sp<GrTextStrike> currStrike;
        const char* text = fFallbackTxt.begin();
        const char* stop = text + fFallbackTxt.count();
        SkPoint* glyphPos = fFallbackPos.begin();
        while (text < stop) {
            const SkGlyph& glyph = glyphCacheProc(cache, &text);
            GrAtlasTextContext::BmpAppendGlyph(blob, runIndex, glyphCache, &currStrike, glyph,
                                               glyphPos->fX, glyphPos->fY, textColor,
                                               cache, textRatio);
            glyphPos++;
        }

        SkGlyphCache::AttachCache(cache);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "GrRenderTargetContext.h"

GR_DRAW_OP_TEST_DEFINE(GrAtlasTextOp) {
    static uint32_t gContextID = SK_InvalidGenID;
    static std::unique_ptr<GrAtlasTextContext> gTextContext;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        gTextContext = GrAtlasTextContext::Make(GrAtlasTextContext::Options());
    }

    // Setup dummy SkPaint / GrPaint / GrRenderTargetContext
    sk_sp<GrRenderTargetContext> rtc(context->makeDeferredRenderTargetContext(
        SkBackingFit::kApprox, 1024, 1024, kRGBA_8888_GrPixelConfig, nullptr));

    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);

    // Because we the GrTextUtils::Paint requires an SkPaint for font info, we ignore the GrPaint
    // param.
    SkPaint skPaint;
    skPaint.setColor(random->nextU());
    skPaint.setLCDRenderText(random->nextBool());
    skPaint.setAntiAlias(skPaint.isLCDRenderText() ? true : random->nextBool());
    skPaint.setSubpixelText(random->nextBool());
    GrTextUtils::Paint utilsPaint(&skPaint, &rtc->colorSpaceInfo());

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

    auto glyphCache = context->contextPriv().getGlyphCache();
    auto restrictedAtlasManager = context->contextPriv().getRestrictedAtlasManager();

    // right now we don't handle textblobs, nor do we handle drawPosText. Since we only intend to
    // test the text op with this unit test, that is okay.
    sk_sp<GrAtlasTextBlob> blob(gTextContext->makeDrawTextBlob(
            context->contextPriv().getTextBlobCache(), glyphCache,
            *context->caps()->shaderCaps(), utilsPaint,
            GrAtlasTextContext::kTextBlobOpScalerContextFlags, viewMatrix, gSurfaceProps, text,
            static_cast<size_t>(textLen), x, y));

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, utilsPaint, gSurfaceProps,
                             gTextContext->dfAdjustTable(), restrictedAtlasManager,
                             rtc->textTarget());
}

#endif
