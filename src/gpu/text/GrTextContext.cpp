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

bool glyph_too_big_for_atlas(const SkGlyph& glyph) {
    return GrDrawOpAtlas::GlyphTooLargeForAtlas(glyph.fWidth, glyph.fHeight);
}

static SkRect rect_to_draw(
        const SkGlyph& glyph, SkPoint origin, SkScalar textScale, GrGlyph::MaskStyle maskStyle) {

    SkScalar dx = SkIntToScalar(glyph.fLeft);
    SkScalar dy = SkIntToScalar(glyph.fTop);
    SkScalar width = SkIntToScalar(glyph.fWidth);
    SkScalar height = SkIntToScalar(glyph.fHeight);

    if (maskStyle == GrGlyph::kDistance_MaskStyle) {
        dx += SK_DistanceFieldInset;
        dy += SK_DistanceFieldInset;
        width -= 2 * SK_DistanceFieldInset;
        height -= 2 * SK_DistanceFieldInset;
    }

    dx *= textScale;
    dy *= textScale;
    width *= textScale;
    height *= textScale;

    return SkRect::MakeXYWH(origin.x() + dx, origin.y() + dy, width, height);
}

void GrTextContext::regenerateGlyphRunList(GrTextBlob* cacheBlob,
                                           GrGlyphCache* glyphCache,
                                           const GrShaderCaps& shaderCaps,
                                           const GrTextUtils::Paint& paint,
                                           SkScalerContextFlags scalerContextFlags,
                                           const SkMatrix& viewMatrix,
                                           const SkSurfaceProps& props,
                                           const SkGlyphRunList& glyphRunList,
                                           SkGlyphRunListDrawer* glyphDrawer) {
    SkPoint origin = glyphRunList.origin();
    cacheBlob->initReusableBlob(paint.luminanceColor(), viewMatrix, origin.x(), origin.y());

    // Regenerate GrTextBlob
    GrTextUtils::RunPaint runPaint(&paint);
    int runIndex = 0;
    for (const auto& glyphRun : glyphRunList) {
        cacheBlob->push_back_run(runIndex);

        if (!runPaint.modifyForRun([glyphRun](SkPaint* p) { *p = glyphRun.paint(); })) {
            continue;
        }
        cacheBlob->setRunPaintFlags(runIndex, runPaint.skPaint().getFlags());

        if (CanDrawAsDistanceFields(runPaint, viewMatrix, props,
                                    shaderCaps.supportsDistanceFieldText(), fOptions)) {
            bool hasWCoord = viewMatrix.hasPerspective()
                             || fOptions.fDistanceFieldVerticesAlwaysHaveW;

            // Setup distance field runPaint and text ratio
            SkScalar textRatio;
            SkPaint dfPaint(runPaint);
            SkScalerContextFlags flags;
            InitDistanceFieldPaint(cacheBlob, &dfPaint, viewMatrix, fOptions, &textRatio, &flags);
            cacheBlob->setHasDistanceField();
            cacheBlob->setSubRunHasDistanceFields(runIndex, runPaint.skPaint().isLCDRenderText(),
                                                  runPaint.skPaint().isAntiAlias(), hasWCoord);

            FallbackGlyphRunHelper fallbackTextHelper(
                    viewMatrix, runPaint, glyphCache->getGlyphSizeLimit(), textRatio);

            sk_sp<GrTextStrike> currStrike;

            {
                auto cache = cacheBlob->setupCache(runIndex, props, flags, dfPaint, nullptr);

                const SkPoint* positionCursor = glyphRun.positions().data();
                for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
                    const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphID);
                    SkPoint glyphPos = origin + *positionCursor++;
                    if (glyph.fWidth > 0) {
                        if (glyph.fMaskFormat == SkMask::kSDF_Format) {

                            SkScalar sx = glyphPos.fX,
                                     sy = glyphPos.fY;

                            if (glyph_too_big_for_atlas(glyph)) {
                                SkRect glyphRect =
                                        rect_to_draw(glyph, glyphPos, textRatio,
                                                     GrGlyph::kDistance_MaskStyle);
                                if (!glyphRect.isEmpty()) {
                                    const SkPath* glyphPath = cache->findPath(glyph);
                                    if (glyphPath != nullptr) {
                                        cacheBlob->appendPathGlyph(
                                                runIndex, *glyphPath, sx, sy, textRatio, false);
                                    }
                                }
                            } else {
                                AppendGlyph(cacheBlob, runIndex, glyphCache, &currStrike,
                                            glyph, GrGlyph::kDistance_MaskStyle, sx, sy,
                                            runPaint.filteredPremulColor(),
                                            cache.get(), textRatio, true);
                            }

                        } else {
                            // can't append non-SDF glyph to SDF batch, send to fallback
                            fallbackTextHelper.appendGlyph(glyph, glyphID, glyphPos);
                        }
                    }
                }
            }

            fallbackTextHelper.drawGlyphs(
                    cacheBlob, runIndex, glyphCache, props, runPaint, scalerContextFlags);

        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            // Ensure the blob is set for bitmaptext
            cacheBlob->setHasBitmap();

            // setup our std runPaint, in hopes of getting hits in the cache
            SkPaint pathPaint(runPaint);
            SkScalar matrixScale = pathPaint.setupForAsPaths();

            FallbackGlyphRunHelper fallbackTextHelper(
                    viewMatrix, runPaint, glyphCache->getGlyphSizeLimit(), matrixScale);

            // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
            pathPaint.setStyle(SkPaint::kFill_Style);
            pathPaint.setPathEffect(nullptr);

            auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
                    pathPaint, &props, SkScalerContextFlags::kFakeGammaAndBoostContrast, nullptr);

            auto drawOnePath =
                    [&fallbackTextHelper, matrixScale, runIndex, cacheBlob]
                    (const SkPath* path, const SkGlyph& glyph, SkPoint position) {
                if (glyph.fMaskFormat == SkMask::kARGB32_Format) {
                    fallbackTextHelper.appendGlyph(glyph, glyph.getGlyphID(), position);
                } else {
                    if (path != nullptr) {
                        cacheBlob->appendPathGlyph(
                                runIndex, *path, position.fX, position.fY, matrixScale, false);
                    }
                }
            };

            glyphDrawer->drawUsingPaths(glyphRun, origin, cache.get(), drawOnePath);

            fallbackTextHelper.drawGlyphs(
                    cacheBlob, runIndex, glyphCache, props, runPaint, scalerContextFlags);

        } else {
            // Ensure the blob is set for bitmaptext
            cacheBlob->setHasBitmap();
            sk_sp<GrTextStrike> currStrike;
            auto cache = cacheBlob->setupCache(
                    runIndex, props, scalerContextFlags, runPaint, &viewMatrix);

            auto perGlyph =
                    [cacheBlob, runIndex, glyphCache, &currStrike, runPaint, cache{cache.get()}]
                    (const SkGlyph& glyph, SkPoint mappedPt) {
                        SkScalar sx = SkScalarFloorToScalar(mappedPt.fX),
                                 sy = SkScalarFloorToScalar(mappedPt.fY);
                        AppendGlyph(cacheBlob, runIndex, glyphCache, &currStrike,
                                    glyph, GrGlyph::kCoverage_MaskStyle, sx, sy,
                                    runPaint.filteredPremulColor(), cache, SK_Scalar1, false);
            };

            auto perPath =
                    [cacheBlob, runIndex]
                    (const SkPath* path, const SkGlyph& glyph, SkPoint position) {
                        SkScalar sx = SkScalarFloorToScalar(position.fX),
                                 sy = SkScalarFloorToScalar(position.fY);
                        cacheBlob->appendPathGlyph(
                                runIndex, *path, sx, sy, SK_Scalar1, true);
            };

            glyphDrawer->drawGlyphRunAsGlyphWithPathFallback(
                    cache.get(), glyphRun, origin, viewMatrix, perGlyph, perPath);
        }
        runIndex += 1;
    }
}

void GrTextContext::AppendGlyph(GrTextBlob* blob, int runIndex,
                                GrGlyphCache* grGlyphCache,
                                sk_sp<GrTextStrike>* strike,
                                const SkGlyph& skGlyph, GrGlyph::MaskStyle maskStyle,
                                SkScalar sx, SkScalar sy,
                                GrColor color, SkGlyphCache* skGlyphCache,
                                SkScalar textRatio, bool needsTransform) {
    if (!*strike) {
        *strike = grGlyphCache->getStrike(skGlyphCache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         maskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, skGlyphCache);
    if (!glyph) {
        return;
    }

    SkASSERT(skGlyph.fWidth == glyph->width());
    SkASSERT(skGlyph.fHeight == glyph->height());

    SkRect glyphRect = rect_to_draw(skGlyph, {sx, sy}, textRatio, maskStyle);

    if (!glyphRect.isEmpty()) {
        blob->appendGlyph(runIndex, glyphRect, color, *strike, glyph, !needsTransform);
    }
}

void GrTextContext::drawGlyphRunList(
        GrContext* context, GrTextUtils::Target* target, const GrClip& clip,
        const SkMatrix& viewMatrix, const SkSurfaceProps& props, const SkGlyphRunList& glyphRunList,
        const SkIRect& clipBounds) {
    SkPoint origin = glyphRunList.origin();

    // Get the first paint to use as the key paint.
    const SkPaint& skPaint = glyphRunList.paint();

    // If we have been abandoned, then don't draw
    if (context->abandoned()) {
        return;
    }

    SkMaskFilterBase::BlurRec blurRec;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = glyphRunList.canCache() && !(skPaint.getPathEffect() ||
                      (mf && !as_MFB(mf)->asABlur(&blurRec)));
    SkScalerContextFlags scalerContextFlags = ComputeScalerContextFlags(target->colorSpaceInfo());

    auto glyphCache = context->contextPriv().getGlyphCache();
    GrTextBlobCache* textBlobCache = context->contextPriv().getTextBlobCache();

    sk_sp<GrTextBlob> cacheBlob;
    GrTextBlob::Key key;
    if (canCache) {
        bool hasLCD = glyphRunList.anyRunsLCD();

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? props.pixelGeometry() :
                                        kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                 ComputeCanonicalColor(skPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = glyphRunList.uniqueID();
        key.fStyle = skPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        key.fScalerContextFlags = scalerContextFlags;
        cacheBlob = textBlobCache->find(key);
    }

    GrTextUtils::Paint paint(&skPaint, &target->colorSpaceInfo());
    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(paint, blurRec, viewMatrix, origin.x(), origin.y())) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            textBlobCache->remove(cacheBlob.get());
            cacheBlob = textBlobCache->makeCachedBlob(glyphRunList, key, blurRec, skPaint);
            this->regenerateGlyphRunList(cacheBlob.get(), glyphCache,
                                     *context->contextPriv().caps()->shaderCaps(), paint,
                                     scalerContextFlags, viewMatrix, props, glyphRunList,
                                     target->glyphDrawer());
        } else {
            textBlobCache->makeMRU(cacheBlob.get());

            if (CACHE_SANITY_CHECK) {
                int glyphCount = glyphRunList.totalGlyphCount();
                int runCount = glyphRunList.runCount();
                sk_sp<GrTextBlob> sanityBlob(textBlobCache->makeBlob(glyphCount, runCount));
                sanityBlob->setupKey(key, blurRec, skPaint);
                this->regenerateGlyphRunList(
                        sanityBlob.get(), glyphCache, *context->contextPriv().caps()->shaderCaps(),
                        paint, scalerContextFlags, viewMatrix, props, glyphRunList,
                        target->glyphDrawer());
                GrTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }
        }
    } else {
        if (canCache) {
            cacheBlob = textBlobCache->makeCachedBlob(glyphRunList, key, blurRec, skPaint);
        } else {
            cacheBlob = textBlobCache->makeBlob(glyphRunList);
        }
        this->regenerateGlyphRunList(cacheBlob.get(), glyphCache,
                                 *context->contextPriv().caps()->shaderCaps(), paint,
                                 scalerContextFlags, viewMatrix, props, glyphRunList,
                                 target->glyphDrawer());
    }

    cacheBlob->flush(target, props, fDistanceAdjustTable.get(), paint,
                     clip, viewMatrix, clipBounds, origin.x(), origin.y());
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

///////////////////////////////////////////////////////////////////////////////////////////////////

void GrTextContext::FallbackGlyphRunHelper::appendGlyph(
        const SkGlyph& glyph, SkGlyphID glyphID, SkPoint glyphPos) {
    SkScalar maxDim = SkTMax(glyph.fWidth, glyph.fHeight)*fTextRatio;
    if (SkScalarNearlyZero(maxDim)) return;

    if (!fUseTransformedFallback) {
        if (!fViewMatrix.isScaleTranslate() || maxDim*fMaxScale > fMaxTextSize) {
            fUseTransformedFallback = true;
            fMaxTextSize -= 2;    // Subtract 2 to account for the bilerp pad around the glyph
        }
    }

    fFallbackTxt.push_back(glyphID);
    if (fUseTransformedFallback) {
        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar glyphTextSize =
                SkTMax(SkScalarFloorToScalar(fTextSize * fMaxTextSize/maxDim), 0.5f*fMaxTextSize);
        fTransformedFallbackTextSize = SkTMin(glyphTextSize, fTransformedFallbackTextSize);
    }
    fFallbackPos.push_back(glyphPos);
}

void GrTextContext::FallbackGlyphRunHelper::drawGlyphs(
        GrTextBlob* blob, int runIndex, GrGlyphCache* glyphCache, const SkSurfaceProps& props,
        const GrTextUtils::Paint& paint, SkScalerContextFlags scalerContextFlags) {
    if (!fFallbackTxt.empty()) {
        blob->initOverride(runIndex);
        blob->setHasBitmap();
        blob->setSubRunHasW(runIndex, fViewMatrix.hasPerspective());
        const SkPaint& skPaint = paint.skPaint();
        SkColor textColor = paint.filteredPremulColor();

        SkScalar textRatio = SK_Scalar1;
        SkPaint fallbackPaint(skPaint);
        SkMatrix matrix = fViewMatrix;
        this->initializeForDraw(&fallbackPaint, &textRatio, &matrix);
        SkExclusiveStrikePtr cache =
                blob->setupCache(runIndex, props, scalerContextFlags, fallbackPaint, &matrix);

        sk_sp<GrTextStrike> currStrike;
        auto glyphPos = fFallbackPos.begin();
        for (auto glyphID : fFallbackTxt) {
            const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphID);
            if (!fUseTransformedFallback) {
                fViewMatrix.mapPoints(&*glyphPos, 1);
                glyphPos->fX = SkScalarFloorToScalar(glyphPos->fX);
                glyphPos->fY = SkScalarFloorToScalar(glyphPos->fY);
            }
            GrTextContext::AppendGlyph(blob, runIndex, glyphCache, &currStrike,
                                          glyph, GrGlyph::kCoverage_MaskStyle,
                                          glyphPos->fX, glyphPos->fY, textColor,
                                          cache.get(), textRatio, fUseTransformedFallback);
            glyphPos++;
        }
    }
}

void GrTextContext::FallbackGlyphRunHelper::initializeForDraw(
        SkPaint* paint, SkScalar* textRatio, SkMatrix* matrix) const {
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

    auto origin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawText(skPaint, text, textLen, origin);


    auto glyphRunList = builder.useGlyphRunList();
    sk_sp<GrTextBlob> blob;
    if (!glyphRunList.empty()) {
        blob = context->contextPriv().getTextBlobCache()->makeBlob(glyphRunList);
        // Use the text and textLen below, because we don't want to mess with the paint.
        SkScalerContextFlags scalerContextFlags =
                ComputeScalerContextFlags(rtc->colorSpaceInfo());
        textContext->regenerateGlyphRunList(
                blob.get(), glyphCache, *context->contextPriv().caps()->shaderCaps(), utilsPaint,
                scalerContextFlags, viewMatrix, surfaceProps, glyphRunList,
                rtc->textTarget()->glyphDrawer());
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
