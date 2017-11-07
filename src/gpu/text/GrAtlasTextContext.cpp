/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"
#include "GrContext.h"
#include "GrTextBlobCache.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkFindAndPlaceGlyph.h"
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
    return CanDrawAsDistanceFields(skPaint, viewMatrix, props, shaderCaps) ||
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

uint32_t GrAtlasTextContext::ComputeScalerContextFlags(const GrColorSpaceInfo& colorSpaceInfo) {
    // If we're doing gamma-correct rendering, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    if (colorSpaceInfo.isGammaCorrect()) {
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
    SkMaskFilter::BlurRec blurRec;
    GrAtlasTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() ||
                      (mf && !mf->asABlur(&blurRec)) ||
                      drawFilter);
    uint32_t scalerContextFlags = ComputeScalerContextFlags(target->colorSpaceInfo());

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

    GrTextUtils::Paint paint(&skPaint, &target->colorSpaceInfo());
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

    cacheBlob->flushCached(context, target, blob, props, fDistanceAdjustTable.get(), paint,
                           drawFilter, clip, viewMatrix, clipBounds, x, y);
}

void GrAtlasTextContext::RegenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            GrAtlasGlyphCache* fontCache,
                                            const GrShaderCaps& shaderCaps,
                                            const GrTextUtils::Paint& paint,
                                            uint32_t scalerContextFlags,
                                            const SkMatrix& viewMatrix,
                                            const SkSurfaceProps& props, const SkTextBlob* blob,
                                            SkScalar x, SkScalar y, SkDrawFilter* drawFilter) {
    cacheBlob->initReusableBlob(paint.luminanceColor(), viewMatrix, x, y);

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
        if (CanDrawAsDistanceFields(runPaint, viewMatrix, props, shaderCaps)) {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    DrawDFText(cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                               viewMatrix, (const char*)it.glyphs(), textLen, x + offset.x(),
                               y + offset.y());
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y + offset.y());
                    DrawDFPosText(cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                                  viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 1,
                                  dfOffset);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y);
                    DrawDFPosText(cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                                  viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 2,
                                  dfOffset);
                    break;
                }
            }
        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            cacheBlob->setRunDrawAsPaths(run);
        } else {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning:
                    DrawBmpText(cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                                viewMatrix, (const char*)it.glyphs(), textLen, x + offset.x(),
                                y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    DrawBmpPosText(cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                                   viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 1,
                                   SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    DrawBmpPosText(cacheBlob, run, fontCache, props, runPaint, scalerContextFlags,
                                   viewMatrix, (const char*)it.glyphs(), textLen, it.pos(), 2,
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
    if (!glyphCount) {
        return nullptr;
    }
    sk_sp<GrAtlasTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, x, y);

    if (CanDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps)) {
        DrawDFText(blob.get(), 0, fontCache, props, paint, scalerContextFlags, viewMatrix, text,
                   byteLength, x, y);
    } else {
        DrawBmpText(blob.get(), 0, fontCache, props, paint, scalerContextFlags, viewMatrix, text,
                    byteLength, x, y);
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
    if (!glyphCount) {
        return nullptr;
    }

    sk_sp<GrAtlasTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, offset.x(), offset.y());

    if (CanDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps)) {
        DrawDFPosText(blob.get(), 0, fontCache, props, paint, scalerContextFlags, viewMatrix, text,
                      byteLength, pos, scalarsPerPosition, offset);
    } else {
        DrawBmpPosText(blob.get(), 0, fontCache, props, paint, scalerContextFlags, viewMatrix, text,
                       byteLength, pos, scalarsPerPosition, offset);
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
    GrTextUtils::Paint paint(&skPaint, &target->colorSpaceInfo());
    if (this->canDraw(skPaint, viewMatrix, props, *context->caps()->shaderCaps())) {
        sk_sp<GrAtlasTextBlob> blob(
                MakeDrawTextBlob(context->getTextBlobCache(), context->getAtlasGlyphCache(),
                                 *context->caps()->shaderCaps(), paint,
                                 ComputeScalerContextFlags(target->colorSpaceInfo()), viewMatrix,
                                 props, text, byteLength, x, y));
        if (blob) {
            blob->flushThrowaway(context, target, props, fDistanceAdjustTable.get(), paint, clip,
                                 viewMatrix, regionClipBounds, x, y);
        }
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawTextAsPath(context, target, clip, paint, viewMatrix, text, byteLength, x, y,
                                regionClipBounds);
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
    } else if (this->canDraw(skPaint, viewMatrix, props, *context->caps()->shaderCaps())) {
        sk_sp<GrAtlasTextBlob> blob(
                MakeDrawPosTextBlob(context->getTextBlobCache(), context->getAtlasGlyphCache(),
                                    *context->caps()->shaderCaps(), paint,
                                    ComputeScalerContextFlags(target->colorSpaceInfo()), viewMatrix,
                                    props, text, byteLength, pos, scalarsPerPosition, offset));
        if (blob) {
            blob->flushThrowaway(context, target, props, fDistanceAdjustTable.get(), paint, clip,
                                 viewMatrix, regionClipBounds, offset.fX, offset.fY);
        }
        return;
    }

    // fall back to drawing as a path
    GrTextUtils::DrawPosTextAsPath(context, target, props, clip, paint, viewMatrix, text,
                                   byteLength, pos, scalarsPerPosition, offset, regionClipBounds);
}

void GrAtlasTextContext::DrawBmpText(GrAtlasTextBlob* blob, int runIndex,
                                     GrAtlasGlyphCache* fontCache, const SkSurfaceProps& props,
                                     const GrTextUtils::Paint& paint, uint32_t scalerContextFlags,
                                     const SkMatrix& viewMatrix, const char text[],
                                     size_t byteLength, SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // Ensure the blob is set for bitmaptext
    blob->setHasBitmap();

    GrAtlasTextStrike* currStrike = nullptr;

    SkGlyphCache* cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);
    SkFindAndPlaceGlyph::ProcessText(paint.skPaint().getTextEncoding(), text, byteLength, {x, y},
                                     viewMatrix, paint.skPaint().getTextAlign(), cache,
                                     [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
                                         position += rounding;
                                         BmpAppendGlyph(blob, runIndex, fontCache, &currStrike,
                                                        glyph, SkScalarFloorToInt(position.fX),
                                                        SkScalarFloorToInt(position.fY),
                                                        paint.filteredPremulColor(), cache);
                                     });

    SkGlyphCache::AttachCache(cache);
}

void GrAtlasTextContext::DrawBmpPosText(GrAtlasTextBlob* blob, int runIndex,
                                        GrAtlasGlyphCache* fontCache, const SkSurfaceProps& props,
                                        const GrTextUtils::Paint& paint,
                                        uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
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

    GrAtlasTextStrike* currStrike = nullptr;

    SkGlyphCache* cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);

    SkFindAndPlaceGlyph::ProcessPosText(
            paint.skPaint().getTextEncoding(), text, byteLength, offset, viewMatrix, pos,
            scalarsPerPosition, paint.skPaint().getTextAlign(), cache,
            [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
                position += rounding;
                BmpAppendGlyph(blob, runIndex, fontCache, &currStrike, glyph,
                               SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                               paint.filteredPremulColor(), cache);
            });

    SkGlyphCache::AttachCache(cache);
}

void GrAtlasTextContext::BmpAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                        GrAtlasGlyphCache* fontCache, GrAtlasTextStrike** strike,
                                        const SkGlyph& skGlyph, int vx, int vy, GrColor color,
                                        SkGlyphCache* cache) {
    if (!*strike) {
        *strike = fontCache->getStrike(cache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kCoverage_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, cache);
    if (!glyph) {
        return;
    }

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);

    blob->appendGlyph(runIndex, r, color, *strike, glyph, cache, skGlyph, SkIntToScalar(vx),
                      SkIntToScalar(vy), 1.0f, true);
}

static const int kMinDFFontSize = 18;
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;
#ifdef SK_BUILD_FOR_ANDROID
static const int kLargeDFFontLimit = 384;
#else
static const int kLargeDFFontLimit = 2 * kLargeDFFontSize;
#endif

bool GrAtlasTextContext::CanDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                                 const SkSurfaceProps& props,
                                                 const GrShaderCaps& caps) {
    if (!viewMatrix.hasPerspective()) {
        SkScalar maxScale = viewMatrix.getMaxScale();
        SkScalar scaledTextSize = maxScale * skPaint.getTextSize();
        // Hinted text looks far better at small resolutions
        // Scaling up beyond 2x yields undesireable artifacts
        if (scaledTextSize < kMinDFFontSize || scaledTextSize > kLargeDFFontLimit) {
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

    // rasterizers and mask filters modify alpha, which doesn't
    // translate well to distance
    if (skPaint.getRasterizer() || skPaint.getMaskFilter() || !caps.shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

void GrAtlasTextContext::InitDistanceFieldPaint(GrAtlasTextBlob* blob,
                                                SkPaint* skPaint,
                                                SkScalar* textRatio,
                                                const SkMatrix& viewMatrix) {
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
        dfMaskScaleFloor = kMinDFFontSize;
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
        dfMaskScaleCeil = kLargeDFFontLimit;
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

void GrAtlasTextContext::DrawDFText(GrAtlasTextBlob* blob, int runIndex,
                                    GrAtlasGlyphCache* fontCache, const SkSurfaceProps& props,
                                    const GrTextUtils::Paint& paint, uint32_t scalerContextFlags,
                                    const SkMatrix& viewMatrix, const char text[],
                                    size_t byteLength, SkScalar x, SkScalar y) {
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
    skPaint.getScalerContextDescriptor(&effects, &desc, props, SkPaint::kNone_ScalerContextFlags,
                                       nullptr);
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

    DrawDFPosText(blob, runIndex, fontCache, props, paint, scalerContextFlags, viewMatrix, text,
                  byteLength, positions.begin(), 2, offset);
}

void GrAtlasTextContext::DrawDFPosText(GrAtlasTextBlob* blob, int runIndex,
                                       GrAtlasGlyphCache* fontCache, const SkSurfaceProps& props,
                                       const GrTextUtils::Paint& paint, uint32_t scalerContextFlags,
                                       const SkMatrix& viewMatrix, const char text[],
                                       size_t byteLength, const SkScalar pos[],
                                       int scalarsPerPosition, const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    SkTDArray<char> fallbackTxt;
    SkTDArray<SkScalar> fallbackPos;

    // Setup distance field paint and text ratio
    SkScalar textRatio;
    SkPaint dfPaint(paint);
    GrAtlasTextContext::InitDistanceFieldPaint(blob, &dfPaint, &textRatio, viewMatrix);
    blob->setHasDistanceField();
    blob->setSubRunHasDistanceFields(runIndex, paint.skPaint().isLCDRenderText(),
                                     paint.skPaint().isAntiAlias());

    GrAtlasTextStrike* currStrike = nullptr;

    // We apply the fake-gamma by altering the distance in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkGlyphCache* cache =
            blob->setupCache(runIndex, props, SkPaint::kNone_ScalerContextFlags, dfPaint, nullptr);
    SkPaint::GlyphCacheProc glyphCacheProc =
            SkPaint::GetGlyphCacheProc(dfPaint.getTextEncoding(), dfPaint.isDevKernText(), true);

    const char* stop = text + byteLength;

    if (SkPaint::kLeft_Align == dfPaint.getTextAlign()) {
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                if (!DfAppendGlyph(blob, runIndex, fontCache, &currStrike, glyph, x, y,
                                   paint.filteredPremulColor(), cache, textRatio, viewMatrix)) {
                    // couldn't append, send to fallback
                    fallbackTxt.append(SkToInt(text - lastText), lastText);
                    *fallbackPos.append() = pos[0];
                    if (2 == scalarsPerPosition) {
                        *fallbackPos.append() = pos[1];
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    } else {
        SkScalar alignMul =
                SkPaint::kCenter_Align == dfPaint.getTextAlign() ? SK_ScalarHalf : SK_Scalar1;
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                SkScalar advanceX = SkFloatToScalar(glyph.fAdvanceX) * alignMul * textRatio;
                SkScalar advanceY = SkFloatToScalar(glyph.fAdvanceY) * alignMul * textRatio;

                if (!DfAppendGlyph(blob, runIndex, fontCache, &currStrike, glyph, x - advanceX,
                                   y - advanceY, paint.filteredPremulColor(), cache, textRatio,
                                   viewMatrix)) {
                    // couldn't append, send to fallback
                    fallbackTxt.append(SkToInt(text - lastText), lastText);
                    *fallbackPos.append() = pos[0];
                    if (2 == scalarsPerPosition) {
                        *fallbackPos.append() = pos[1];
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    }

    SkGlyphCache::AttachCache(cache);
    if (fallbackTxt.count()) {
        blob->initOverride(runIndex);
        GrAtlasTextContext::DrawBmpPosText(blob, runIndex, fontCache, props, paint,
                                           scalerContextFlags, viewMatrix, fallbackTxt.begin(),
                                           fallbackTxt.count(), fallbackPos.begin(),
                                           scalarsPerPosition, offset);
    }
}

bool GrAtlasTextContext::DfAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                       GrAtlasGlyphCache* cache, GrAtlasTextStrike** strike,
                                       const SkGlyph& skGlyph, SkScalar sx, SkScalar sy,
                                       GrColor color, SkGlyphCache* glyphCache, SkScalar textRatio,
                                       const SkMatrix& viewMatrix) {
    if (!*strike) {
        *strike = cache->getStrike(glyphCache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kDistance_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, glyphCache);
    if (!glyph) {
        return true;
    }

    // fallback to color glyph support
    if (kA8_GrMaskFormat != glyph->fMaskFormat) {
        return false;
    }

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft + SK_DistanceFieldInset);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop + SK_DistanceFieldInset);
    SkScalar width = SkIntToScalar(glyph->fBounds.width() - 2 * SK_DistanceFieldInset);
    SkScalar height = SkIntToScalar(glyph->fBounds.height() - 2 * SK_DistanceFieldInset);

    SkScalar scale = textRatio;
    dx *= scale;
    dy *= scale;
    width *= scale;
    height *= scale;
    sx += dx;
    sy += dy;
    SkRect glyphRect = SkRect::MakeXYWH(sx, sy, width, height);

    blob->appendGlyph(runIndex, glyphRect, color, *strike, glyph, glyphCache, skGlyph, sx - dx,
                      sy - dy, scale, false);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "GrRenderTargetContext.h"

GR_DRAW_OP_TEST_DEFINE(GrAtlasTextOp) {
    static uint32_t gContextID = SK_InvalidGenID;
    static GrAtlasTextContext* gTextContext = nullptr;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        delete gTextContext;

        gTextContext = GrAtlasTextContext::Create();
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

    // right now we don't handle textblobs, nor do we handle drawPosText. Since we only intend to
    // test the text op with this unit test, that is okay.
    sk_sp<GrAtlasTextBlob> blob(GrAtlasTextContext::MakeDrawTextBlob(
            context->getTextBlobCache(), context->getAtlasGlyphCache(),
            *context->caps()->shaderCaps(), utilsPaint,
            GrAtlasTextContext::kTextBlobOpScalerContextFlags, viewMatrix, gSurfaceProps, text,
            static_cast<size_t>(textLen), x, y));

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, utilsPaint, gSurfaceProps,
                             gTextContext->dfAdjustTable(), context->getAtlasGlyphCache(),
                             rtc->textTarget());
}

#endif
