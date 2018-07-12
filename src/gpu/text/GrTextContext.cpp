/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextContext.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrSDFMaskFilter.h"
#include "GrTextBlobCache.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphRun.h"
#include "SkGr.h"
#include "SkGraphics.h"
#include "SkMakeUnique.h"
#include "SkMaskFilterBase.h"
#include "SkPaintPriv.h"
#include "SkTextMapStateProc.h"
#include "SkTo.h"
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

GrTextContext::GrTextContext(const Options& options)
        : fDistanceAdjustTable(new GrDistanceFieldAdjustTable), fOptions(options) {
    SanitizeOptions(&fOptions);
}

std::unique_ptr<GrTextContext> GrTextContext::Make(const Options& options) {
    return std::unique_ptr<GrTextContext>(new GrTextContext(options));
}

SkColor GrTextContext::ComputeCanonicalColor(const SkPaint& paint, bool lcd) {
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

SkScalerContextFlags GrTextContext::ComputeScalerContextFlags(
        const GrColorSpaceInfo& colorSpaceInfo) {
    // If we're doing linear blending, then we can disable the gamma hacks.
    // Otherwise, leave them on. In either case, we still want the contrast boost:
    // TODO: Can we be even smarter about mask gamma based on the dest transfer function?
    if (colorSpaceInfo.isLinearlyBlended()) {
        return SkScalerContextFlags::kBoostContrast;
    } else {
        return SkScalerContextFlags::kFakeGammaAndBoostContrast;
    }
}

// TODO if this function ever shows up in profiling, then we can compute this value when the
// textblob is being built and cache it.  However, for the time being textblobs mostly only have 1
// run so this is not a big deal to compute here.
bool GrTextContext::HasLCD(const SkTextBlob* blob) {
    SkTextBlobRunIterator it(blob);
    for (; !it.done(); it.next()) {
        if (it.isLCD()) {
            return true;
        }
    }
    return false;
}

void GrTextContext::drawTextBlob(GrContext* context, GrTextUtils::Target* target,
                                 const GrClip& clip, const SkPaint& skPaint,
                                 const SkMatrix& viewMatrix, const SkSurfaceProps& props,
                                 const SkTextBlob* blob, SkScalar x, SkScalar y,
                                 const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (context->abandoned()) {
        return;
    }

    sk_sp<GrTextBlob> cacheBlob;
    SkMaskFilterBase::BlurRec blurRec;
    GrTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() || (mf && !as_MFB(mf)->asABlur(&blurRec)));
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(target->colorSpaceInfo());

    auto glyphCache = context->contextPriv().getGlyphCache();
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
                                     *context->contextPriv().caps()->shaderCaps(), paint,
                                     scalerContextFlags, viewMatrix, props, blob, x, y);
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(glyphCount, runCount));
                sanityBlob->setupKey(key, blurRec, skPaint);
                this->regenerateTextBlob(
                        sanityBlob.get(), glyphCache, *context->contextPriv().caps()->shaderCaps(),
                        paint, scalerContextFlags, viewMatrix, props, blob, x, y);
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(blob, key, blurRec, skPaint);
        } else {
            cacheBlob = textBlobCache->makeBlob(blob);
        }
        this->regenerateTextBlob(cacheBlob.get(), glyphCache,
                                 *context->contextPriv().caps()->shaderCaps(), paint,
                                 scalerContextFlags, viewMatrix, props, blob, x, y);
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), paint,
                     clip, viewMatrix, clipBounds, x, y);
}

void GrTextContext::regenerateTextBlob(GrTextBlob* cacheBlob,
                                       GrGlyphCache* glyphCache,
                                       const GrShaderCaps& shaderCaps,
                                       const GrTextUtils::Paint& paint,
                                       SkScalerContextFlags scalerContextFlags,
                                       const SkMatrix& viewMatrix,
                                       const SkSurfaceProps& props, const SkTextBlob* blob,
                                       SkScalar x, SkScalar y) const {
    cacheBlob->initReusableBlob(paint.luminanceColor(), viewMatrix, x, y);

    // Regenerate textblob
    SkTextBlobRunIterator it(blob);
    GrTextUtils::RunPaint runPaint(&paint);
    for (int run = 0; !it.done(); it.next(), run++) {
        int glyphCount = it.glyphCount();
        size_t textLen = glyphCount * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        cacheBlob->push_back_run(run);
        if (!runPaint.modifyForRun([it](SkPaint* p) { it.applyFontToPaint(p); })) {
            continue;
        }
        cacheBlob->setRunPaintFlags(run, runPaint.skPaint().getFlags());

        if (CanDrawAsDistanceFields(runPaint, viewMatrix, props,
                                    shaderCaps.supportsDistanceFieldText(), fOptions)) {
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    auto origin = SkPoint::Make(x + offset.x(), y + offset.y());
                    SkGlyphRunBuilder builder;
                    builder.drawText(runPaint.skPaint(),
                                     (const char*) it.glyphs(), textLen, origin);

                    auto glyphRunList = builder.useGlyphRunList();
                    if (!glyphRunList->empty()) {
                        auto glyphRun = (*glyphRunList)[0];
                        glyphRun.temporaryShuntToCallback(
                            [&](size_t runSize, const char* glyphIDs, const SkScalar* pos) {
                                this->drawDFPosText(
                                    cacheBlob, run, glyphCache, props, runPaint,
                                    scalerContextFlags, viewMatrix, glyphIDs, 2 * runSize,
                                    pos, 2, SkPoint::Make(0, 0));
                            });
                    }
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
                case SkTextBlob::kDefault_Positioning: {
                    auto origin = SkPoint::Make(x + offset.x(), y + offset.y());
                    SkGlyphRunBuilder builder;
                    builder.drawText(runPaint.skPaint(),
                                     (const char*) it.glyphs(), textLen, origin);

                    auto glyphRunList = builder.useGlyphRunList();
                    if (!glyphRunList->empty()) {
                        auto glyphRun = (*glyphRunList)[0];

                        glyphRun.temporaryShuntToCallback(
                            [&](size_t runSize, const char* glyphIDs, const SkScalar* pos) {
                                this->DrawBmpPosText(
                                    cacheBlob, run, glyphCache, props, runPaint,
                                    scalerContextFlags, viewMatrix, glyphIDs, 2 * runSize,
                                    pos, 2, SkPoint::Make(0, 0));
                            });
                    }
                    break;
                }
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

inline sk_sp<GrTextBlob>
GrTextContext::makeDrawPosTextBlob(GrTextBlobCache* blobCache,
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

    sk_sp<GrTextBlob> blob = blobCache->makeBlob(glyphCount, 1);
    blob->initThrowawayBlob(viewMatrix, offset.x(), offset.y());
    blob->setRunPaintFlags(0, paint.skPaint().getFlags());

    if (CanDrawAsDistanceFields(paint, viewMatrix, props, shaderCaps.supportsDistanceFieldText(),
                                fOptions)) {
        this->drawDFPosText(blob.get(), 0, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                            text, byteLength, pos, scalarsPerPosition, offset);
    } else {
        DrawBmpPosText(blob.get(), 0, glyphCache, props, paint, scalerContextFlags, viewMatrix,
                       text, byteLength, pos, scalarsPerPosition, offset);
    }
    return blob;
}

void GrTextContext::drawPosText(GrContext* context, GrTextUtils::Target* target,
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
    auto textBlobCache = context->contextPriv().getTextBlobCache();

    sk_sp<GrTextBlob> blob(this->makeDrawPosTextBlob(
            textBlobCache, glyphCache, *context->contextPriv().caps()->shaderCaps(), paint,
            ComputeScalerContextFlags(target->colorSpaceInfo()), viewMatrix, props, text,
            byteLength, pos, scalarsPerPosition, offset));
    if (blob) {
        blob->flush(target, props, fDistanceAdjustTable.get(), paint,
                    clip, viewMatrix, regionClipBounds, offset.fX, offset.fY);
    }
}


void GrTextContext::DrawBmpPosText(GrTextBlob* blob, int runIndex,
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
    auto cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);
    SkFindAndPlaceGlyph::ProcessPosText(
            paint.skPaint().getTextEncoding(), text, byteLength, offset, viewMatrix, pos,
            scalarsPerPosition, cache.get(),
            [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
                position += rounding;
                BmpAppendGlyph(blob, runIndex, glyphCache, &currStrike, glyph,
                               SkScalarFloorToScalar(position.fX),
                               SkScalarFloorToScalar(position.fY),
                               paint.filteredPremulColor(), cache.get(), SK_Scalar1, false);
            });
}

void GrTextContext::DrawBmpPosTextAsPaths(GrTextBlob* blob, int runIndex,
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
    FallbackTextHelper fallbackTextHelper(viewMatrix, origPaint, glyphCache->getGlyphSizeLimit(),
                                          matrixScale);

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    pathPaint.setStyle(SkPaint::kFill_Style);
    pathPaint.setPathEffect(nullptr);

    SkPaint::GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(pathPaint.getTextEncoding(),
                                                                        true);
    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
            pathPaint, &props, SkScalerContextFlags::kFakeGammaAndBoostContrast, nullptr);

    const char*        stop = text + byteLength;
    const char*        lastText = text;
    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache.get(), &text);
        if (glyph.fWidth) {
            SkPoint loc;
            tmsProc(pos, &loc);
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

void GrTextContext::BmpAppendGlyph(GrTextBlob* blob, int runIndex,
                                   GrGlyphCache* grGlyphCache,
                                   sk_sp<GrTextStrike>* strike,
                                   const SkGlyph& skGlyph, SkScalar sx, SkScalar sy,
                                   GrColor color, SkGlyphCache* skGlyphCache,
                                   SkScalar textRatio, bool needsTransform) {
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
                      textRatio, !needsTransform);
}

void GrTextContext::SanitizeOptions(Options* options) {
    if (options->fMaxDistanceFieldFontSize < 0.f) {
        options->fMaxDistanceFieldFontSize = kDefaultMaxDistanceFieldFontSize;
    }
    if (options->fMinDistanceFieldFontSize < 0.f) {
        options->fMinDistanceFieldFontSize = kDefaultMinDistanceFieldFontSize;
    }
}

bool GrTextContext::CanDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                            const SkSurfaceProps& props,
                                            bool contextSupportsDistanceFieldText,
                                            const Options& options) {
    if (!viewMatrix.hasPerspective()) {
        SkScalar maxScale = viewMatrix.getMaxScale();
        SkScalar scaledTextSize = maxScale * skPaint.getTextSize();
        // Hinted text looks far better at small resolutions
        // Scaling up beyond 2x yields undesireable artifacts
        if (scaledTextSize < options.fMinDistanceFieldFontSize ||
            scaledTextSize > options.fMaxDistanceFieldFontSize) {
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
    if (skPaint.getMaskFilter() || !contextSupportsDistanceFieldText) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

void GrTextContext::InitDistanceFieldPaint(GrTextBlob* blob,
                                           SkPaint* skPaint,
                                           const SkMatrix& viewMatrix,
                                           const Options& options,
                                           SkScalar* textRatio,
                                           SkScalerContextFlags* flags) {
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
        dfMaskScaleFloor = options.fMinDistanceFieldFontSize;
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
        dfMaskScaleCeil = options.fMaxDistanceFieldFontSize;
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
    if (blob) {
        blob->setMinAndMaxScale(dfMaskScaleFloor / scaledTextSize,
                                dfMaskScaleCeil / scaledTextSize);
    }

    skPaint->setAntiAlias(true);
    skPaint->setLCDRenderText(false);
    skPaint->setAutohinted(false);
    skPaint->setHinting(SkPaint::kNormal_Hinting);
    skPaint->setSubpixelText(true);

    skPaint->setMaskFilter(GrSDFMaskFilter::Make());

    // We apply the fake-gamma by altering the distance in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    *flags = SkScalerContextFlags::kNone;
}

void GrTextContext::drawDFPosText(GrTextBlob* blob, int runIndex,
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

    bool hasWCoord = viewMatrix.hasPerspective() || fOptions.fDistanceFieldVerticesAlwaysHaveW;

    // Setup distance field paint and text ratio
    SkScalar textRatio;
    SkPaint dfPaint(paint);
    SkScalerContextFlags flags;
    InitDistanceFieldPaint(blob, &dfPaint, viewMatrix, fOptions, &textRatio, &flags);
    blob->setHasDistanceField();
    blob->setSubRunHasDistanceFields(runIndex, paint.skPaint().isLCDRenderText(),
                                     paint.skPaint().isAntiAlias(), hasWCoord);

    FallbackTextHelper fallbackTextHelper(viewMatrix, paint, glyphCache->getGlyphSizeLimit(),
                                          textRatio);

    sk_sp<GrTextStrike> currStrike;

    {
        auto cache = blob->setupCache(runIndex, props, flags, dfPaint, nullptr);
        SkPaint::GlyphCacheProc glyphCacheProc =
            SkPaint::GetGlyphCacheProc(dfPaint.getTextEncoding(), true);

        const char* stop = text + byteLength;

        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache.get(), &text);

            if (glyph.fWidth) {
                SkPoint glyphPos(offset);
                glyphPos.fX += pos[0];
                glyphPos.fY += (2 == scalarsPerPosition ? pos[1] : 0);

                if (glyph.fMaskFormat == SkMask::kSDF_Format) {
                    DfAppendGlyph(blob, runIndex, glyphCache, &currStrike, glyph, glyphPos.fX,
                                  glyphPos.fY, paint.filteredPremulColor(), cache.get(), textRatio);
                } else {
                    // can't append non-SDF glyph to SDF batch, send to fallback
                    fallbackTextHelper.appendText(glyph, SkToInt(text - lastText), lastText,
                                                  glyphPos);
                }
            }
            pos += scalarsPerPosition;
        }
    }

    fallbackTextHelper.drawText(blob, runIndex, glyphCache, props, paint, scalerContextFlags);
}

// TODO: merge with BmpAppendGlyph
void GrTextContext::DfAppendGlyph(GrTextBlob* blob, int runIndex,
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

void GrTextContext::FallbackTextHelper::appendText(const SkGlyph& glyph, int count,
                                                        const char* text, SkPoint glyphPos) {
    SkScalar maxDim = SkTMax(glyph.fWidth, glyph.fHeight)*fTextRatio;
    if (SkScalarNearlyZero(maxDim)) return;

    if (!fUseTransformedFallback) {
        if (!fViewMatrix.isScaleTranslate() || maxDim*fMaxScale > fMaxTextSize) {
            fUseTransformedFallback = true;
            fMaxTextSize -= 2;    // Subtract 2 to account for the bilerp pad around the glyph
        }
    }

    fFallbackTxt.append(count, text);
    if (fUseTransformedFallback) {
        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar glyphTextSize = SkTMax(SkScalarFloorToScalar(fTextSize * fMaxTextSize/maxDim),
                                        0.5f*fMaxTextSize);
        fTransformedFallbackTextSize = SkTMin(glyphTextSize, fTransformedFallbackTextSize);
    }
    *fFallbackPos.append() = glyphPos;
}

void GrTextContext::FallbackTextHelper::drawText(GrTextBlob* blob, int runIndex,
                                                 GrGlyphCache* glyphCache,
                                                 const SkSurfaceProps& props,
                                                 const GrTextUtils::Paint& paint,
                                                 SkScalerContextFlags scalerContextFlags) {
    if (fFallbackTxt.count()) {
        blob->initOverride(runIndex);
        blob->setHasBitmap();
        blob->setSubRunHasW(runIndex, fViewMatrix.hasPerspective());
        SkExclusiveStrikePtr cache;
        const SkPaint& skPaint = paint.skPaint();
        SkPaint::GlyphCacheProc glyphCacheProc =
            SkPaint::GetGlyphCacheProc(skPaint.getTextEncoding(), true);
        SkColor textColor = paint.filteredPremulColor();

        SkScalar textRatio = SK_Scalar1;
        SkPaint fallbackPaint(skPaint);
        SkMatrix matrix = fViewMatrix;
        this->initializeForDraw(&fallbackPaint, &textRatio, &matrix);
        cache = blob->setupCache(runIndex, props, scalerContextFlags, fallbackPaint, &matrix);

        sk_sp<GrTextStrike> currStrike;
        const char* text = fFallbackTxt.begin();
        const char* stop = text + fFallbackTxt.count();
        SkPoint* glyphPos = fFallbackPos.begin();
        while (text < stop) {
            const SkGlyph& glyph = glyphCacheProc(cache.get(), &text);
            if (!fUseTransformedFallback) {
                fViewMatrix.mapPoints(glyphPos, 1);
                glyphPos->fX = SkScalarFloorToScalar(glyphPos->fX);
                glyphPos->fY = SkScalarFloorToScalar(glyphPos->fY);
            }
            GrTextContext::BmpAppendGlyph(blob, runIndex, glyphCache, &currStrike, glyph,
                                               glyphPos->fX, glyphPos->fY, textColor,
                                               cache.get(), textRatio, fUseTransformedFallback);
            glyphPos++;
        }
    }
}

void GrTextContext::FallbackTextHelper::initializeForDraw(SkPaint* paint, SkScalar* textRatio,
                                                          SkMatrix* matrix) const {
    if (!fUseTransformedFallback) return;

    paint->setTextSize(fTransformedFallbackTextSize);
    *textRatio = fTextSize / fTransformedFallbackTextSize;
    *matrix = SkMatrix::I();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "GrRenderTargetContext.h"

std::unique_ptr<GrDrawOp> GrTextContext::createOp_TestingOnly(GrContext* context,
                                                              GrTextContext* textContext,
                                                              GrRenderTargetContext* rtc,
                                                              const SkPaint& skPaint,
                                                              const SkMatrix& viewMatrix,
                                                              const char* text,
                                                              int x,
                                                              int y) {
    auto glyphCache = context->contextPriv().getGlyphCache();

    static SkSurfaceProps surfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    size_t textLen = (int)strlen(text);

    GrTextUtils::Paint utilsPaint(&skPaint, &rtc->colorSpaceInfo());

    // right now we don't handle textblobs, nor do we handle drawPosText. Since we only intend to
    // test the text op with this unit test, that is okay.

    auto origin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawText(skPaint, text, textLen, origin);
    sk_sp<GrTextBlob> blob;

    auto glyphRunList = builder.useGlyphRunList();
    if (!glyphRunList->empty()) {
        auto glyphRun = (*glyphRunList)[0];
        // Use the text and textLen below, because we don't want to mess with the paint.
        glyphRun.temporaryShuntToCallback(
            [&](size_t runSize, const char* glyphIDs, const SkScalar* pos) {
                blob = textContext->makeDrawPosTextBlob(
                    context->contextPriv().getTextBlobCache(), glyphCache,
                    *context->contextPriv().caps()->shaderCaps(), utilsPaint,
                    GrTextContext::kTextBlobOpScalerContextFlags, viewMatrix, surfaceProps,
                    text,
                    textLen, pos, 2, origin);
            });
    }

    return blob->test_makeOp(textLen, 0, 0, viewMatrix, x, y, utilsPaint, surfaceProps,
                             textContext->dfAdjustTable(), rtc->textTarget());
}

GR_DRAW_OP_TEST_DEFINE(GrAtlasTextOp) {
    static uint32_t gContextID = SK_InvalidGenID;
    static std::unique_ptr<GrTextContext> gTextContext;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        gTextContext = GrTextContext::Make(GrTextContext::Options());
    }

    // Setup dummy SkPaint / GrPaint / GrRenderTargetContext
    sk_sp<GrRenderTargetContext> rtc(context->contextPriv().makeDeferredRenderTargetContext(
        SkBackingFit::kApprox, 1024, 1024, kRGBA_8888_GrPixelConfig, nullptr));

    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);

    // Because we the GrTextUtils::Paint requires an SkPaint for font info, we ignore the GrPaint
    // param.
    SkPaint skPaint;
    skPaint.setColor(random->nextU());
    skPaint.setLCDRenderText(random->nextBool());
    skPaint.setAntiAlias(skPaint.isLCDRenderText() ? true : random->nextBool());
    skPaint.setSubpixelText(random->nextBool());

    const char* text = "The quick brown fox jumps over the lazy dog.";

    // create some random x/y offsets, including negative offsets
    static const int kMaxTrans = 1024;
    int xPos = (random->nextU() % 2) * 2 - 1;
    int yPos = (random->nextU() % 2) * 2 - 1;
    int xInt = (random->nextU() % kMaxTrans) * xPos;
    int yInt = (random->nextU() % kMaxTrans) * yPos;

    return gTextContext->createOp_TestingOnly(context, gTextContext.get(), rtc.get(),
                                              skPaint, viewMatrix, text, xInt, yInt);
}

#endif
