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
// Fallback uses a fast strategy and a general strategy. If the viewMatrix is only scale and
// translate, and all the glyphs in the run are small enough, then the font use is just scaled up
// from the canonical size to the display size, and the positions are all transformed. Otherwise,
// a suitable font size is found, and the view transformation happens during rendering. The font
// in the general case is oriented with x and y, and all rotations, scaling, etc. happen during
// rendering.
template <typename Fallback>
static void process_ARGB_fallback(
        SkGlyphCache* pathCache,
        const SkMatrix& viewMatrix,
        const SkSurfaceProps& props,
        SkScalerContextFlags scalerContextFlags,
        int runIndex,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkPoint> positions,
        const SkPaint& runPaint,
        SkScalar textRatio,
        GrTextBlob* blob,
        Fallback fallback) {
    if (glyphIDs.empty()) { return; }

    // Find the largest width or height among all the paths.
    // TODO: move this to initial pass for selecting fallback glyphs
    SkScalar maxGlyphDimension{-SK_ScalarInfinity};
    for (auto glyphID : glyphIDs) {
        const SkGlyph& glyph = pathCache->getGlyphMetrics(glyphID, {0, 0});
        auto glyphMax = std::max(glyph.fWidth, glyph.fHeight);
        maxGlyphDimension = std::max(maxGlyphDimension, (SkScalar)glyphMax);
    }

    // The common case is the glyphs only need to be scaled and translated. In that case do the
    // scale and translate here in fallback. A problem is that if the maximum glyph dimension is
    // too large then the code can not do the scale and translate in fallback, and the needs use
    // the full transform machinery.
    SkScalar maxScale = viewMatrix.getMaxScale();
    SkScalar fallbackTextScale;
    // This is a conservative estimate of the longest dimension among all the glyph widths and
    // heights.
    // TODO: conservativeMaxGlyphDimension is calculated from the path font cache, and the
    // scaled, this may not result in the same dimensions as using the fallbackCache below.
    // Should this be more conservative?
    SkScalar conservativeMaxGlyphDimension = maxGlyphDimension * textRatio * maxScale;

    const SkPoint* finalPositions;
    bool useFastPath =
            viewMatrix.isScaleTranslate() && conservativeMaxGlyphDimension <= maxGlyphDimension;
    if (useFastPath) {
        std::vector<SkPoint> transformedPositions{positions.begin(), positions.end()};
        viewMatrix.mapPoints(transformedPositions.data(), transformedPositions.size());
        for (SkPoint& point : transformedPositions) {
            point.fX =  SkScalarFloorToScalar(point.fX);
            point.fY =  SkScalarFloorToScalar(point.fY);
        }
        finalPositions = transformedPositions.data();
        fallbackTextScale = SK_Scalar1;
        fallback(runPaint, glyphIDs, transformedPositions, SK_Scalar1, viewMatrix);

    } else {

        // Subtract 2 to account for the bilerp pad around the glyph
        SkScalar maxAtlasDimension = SkGlyphCacheCommon::kSkSideTooBigForAtlas - 2;

        SkScalar runPaintTextSize = runPaint.getTextSize();

        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar fallbackTextSize =
                std::min((maxAtlasDimension / conservativeMaxGlyphDimension) * runPaintTextSize,
                         0.5f * runPaintTextSize);

        SkPaint fallbackPaint{runPaint};
        fallbackPaint.setTextSize(SkScalarFloorToScalar(fallbackTextSize));

        fallbackCache = blob->setupCache(
                runIndex, props, scalerContextFlags, fallbackPaint, &SkMatrix::I());

        finalPositions = positions.data();
        fallback(fallbackPaint, glyphIDs, positions, SK_Scalar1, SkMatrix::I());
    }



    for (SkGlyphID glyphID : glyphIDs) {
        const SkGlyph& glyph = fallbackCache->getGlyphIDMetrics(glyphID);
        GrTextContext::AppendGlyph(blob, runIndex, glyphCache, &currStrike,
                                   glyph, GrGlyph::kCoverage_MaskStyle,
                                   glyphPos->fX, glyphPos->fY, textColor,
                                   cache.get(), textRatio, fUseTransformedFallback);
        glyphPos++;
    }

    SkPaint fallbackPaint(runPaint);
    SkScalar textRatio = SK_Scalar1;
    SkMatrix matrix = viewMatrix;
    SkScalar textSize = runPaint.getTextSize();
    SkScalar maxScale = viewMatrix.getMaxScale();

    bool useFullTransform = false;
    SkScalar maxTextSize = SkGlyphCacheCommon::kSkSideTooBigForAtlas;
    SkScalar scaledMaxGlyphDimension = maxGlyphDimension * textRatio * maxScale;

    if (!viewMatrix.isScaleTranslate() || scaledMaxGlyphDimension > scaledMaxGlyphDimension) {
        // If the code will transform, then populate the cache with aligned to x and y because
        // the transform will be applied later. Carefully choose the size so that the largest
        // size will fit in kSkSideTooBigForAtlas.
        useFullTransform = true;


        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar glyphTextSize =
                SkTMax(SkScalarFloorToScalar(
                               textSize * maxTextSize / maxDim), 0.5f * maxTextSize);
        transformedFallbackTextSize = SkTMin(glyphTextSize, transformedFallbackTextSize);
        fallbackPaint.setTextSize(fTransformedFallbackTextSize);
        textRatio = textSize / fTransformedFallbackTextSize;
        matrix = SkMatrix::I();
    }



    for (auto glyphID : glyphIDs) {
        const SkGlyph& glyph = runCache->getGlyphMetrics(glyphID, {0, 0});
        SkScalar maxDim = SkTMax(glyph.fWidth, glyph.fHeight) * textRatio;
        if (SkScalarNearlyZero(maxDim)) {
            continue;
        }

        if (!useFullTransform) {
            if (!viewMatrix.isScaleTranslate() || maxDim * maxScale > maxTextSize) {
                useFullTransform = true;
                maxTextSize -= 2;    // Subtract 2 to account for the bilerp pad around the glyph

            }
        }

        if (!useFullTransform) {
            // If there's a glyph in the font that's particularly large, it's possible
            // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
            // that glyph than make the others blurry, so we set a minimum size of half the
            // maximum text size to avoid this case.
            SkScalar glyphTextSize =
                    SkTMax(SkScalarFloorToScalar(textSize * textRatio/maxDim), 0.5f * maxTextSize);
            transformedFallbackTextSize = SkTMin(glyphTextSize, transformedFallbackTextSize);
        }

        if (useFullTransform) {
            // If there's a glyph in the font that's particularly large, it's possible
            // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
            // that glyph than make the others blurry, so we set a minimum size of half the
            // maximum text size to avoid this case.
            SkScalar glyphTextSize =
                    SkTMax(SkScalarFloorToScalar(
                            textSize * maxTextSize / maxDim), 0.5f * maxTextSize);
            transformedFallbackTextSize = SkTMin(glyphTextSize, transformedFallbackTextSize);
        }
    }


}

GrTextContext::FallbackGlyphRunHelper::FallbackGlyphRunHelper(
        const SkMatrix& viewMatrix, const SkPaint& runPaint, SkScalar textRatio)
        : fViewMatrix(viewMatrix)
        , fTextSize(runPaint.getTextSize())
        , fMaxTextSize(SkGlyphCacheCommon::kSkSideTooBigForAtlas)
        , fTextRatio(textRatio)
        , fTransformedFallbackTextSize(fMaxTextSize)
        , fMaxScale(viewMatrix.getMaxScale())
        , fUseTransformedFallback(false) { }

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

    if (fUseTransformedFallback) {
        // If there's a glyph in the font that's particularly large, it's possible
        // that fScaledFallbackTextSize may end up minimizing too much. We'd rather skip
        // that glyph than make the others blurry, so we set a minimum size of half the
        // maximum text size to avoid this case.
        SkScalar glyphTextSize =
                SkTMax(SkScalarFloorToScalar(fTextSize * fMaxTextSize/maxDim), 0.5f*fMaxTextSize);
        fTransformedFallbackTextSize = SkTMin(glyphTextSize, fTransformedFallbackTextSize);
    }
    fFallbackTxt.push_back(glyphID);
    fFallbackPos.push_back(glyphPos);
}

void GrTextContext::FallbackGlyphRunHelper::drawGlyphs(
        GrTextBlob* blob, int runIndex, GrGlyphCache* glyphCache, const SkSurfaceProps& props,
        const SkPaint& runPaint, GrColor filteredColor, SkScalerContextFlags scalerContextFlags) {
    if (!fFallbackTxt.empty()) {
        blob->initOverride(runIndex);
        blob->setHasBitmap();
        blob->setSubRunHasW(runIndex, fViewMatrix.hasPerspective());
        SkColor textColor = filteredColor;

        SkPaint fallbackPaint(runPaint);
        SkScalar textRatio = SK_Scalar1;
        SkMatrix matrix = fViewMatrix;

        //this->initializeForDraw(&fallbackPaint, &textRatio, &matrix);
        if (fUseTransformedFallback) {
            fallbackPaint.setTextSize(fTransformedFallbackTextSize);
            textRatio = fTextSize / fTransformedFallbackTextSize;
            matrix = SkMatrix::I();
        }
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
    if (fUseTransformedFallback) {
        paint->setTextSize(fTransformedFallbackTextSize);
        *textRatio = fTextSize / fTransformedFallbackTextSize;
        *matrix = SkMatrix::I();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////


#if GR_TEST_UTILS

#include "GrRenderTargetContext.h"

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
