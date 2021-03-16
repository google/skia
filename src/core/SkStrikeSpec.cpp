/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeSpec.h"

#include "include/core/SkGraphics.h"
#include "src/core/SkDraw.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTLazy.h"

#if SK_SUPPORT_GPU
#include "src/gpu/text/GrSDFMaskFilter.h"
#include "src/gpu/text/GrSDFTControl.h"
#include "src/gpu/text/GrStrikeCache.h"
#endif

SkStrikeSpec SkStrikeSpec::MakeMask(const SkFont& font, const SkPaint& paint,
                                    const SkSurfaceProps& surfaceProps,
                                    SkScalerContextFlags scalerContextFlags,
                                    const SkMatrix& deviceMatrix) {

    return SkStrikeSpec(font, paint, surfaceProps, scalerContextFlags, deviceMatrix, 1);
}

SkStrikeSpec SkStrikeSpec::MakePath(const SkFont& font, const SkPaint& paint,
                                    const SkSurfaceProps& surfaceProps,
                                    SkScalerContextFlags scalerContextFlags) {

    // setup our std runPaint, in hopes of getting hits in the cache
    SkPaint pathPaint{paint};
    SkFont pathFont{font};

    // The factor to get from the size stored in the strike to the size needed for
    // the source.
    SkScalar strikeToSourceRatio = pathFont.setupForAsPaths(&pathPaint);

    // The sub-pixel position will always happen when transforming to the screen.
    pathFont.setSubpixel(false);

    return SkStrikeSpec(pathFont, pathPaint, surfaceProps, scalerContextFlags,
                        SkMatrix::I(), strikeToSourceRatio);
}

SkStrikeSpec SkStrikeSpec::MakeSourceFallback(
        const SkFont& font,
        const SkPaint& paint,
        const SkSurfaceProps& surfaceProps,
        SkScalerContextFlags scalerContextFlags,
        SkScalar maxSourceGlyphDimension) {

    // Subtract 2 to account for the bilerp pad around the glyph
    SkScalar maxAtlasDimension = SkStrikeCommon::kSkSideTooBigForAtlas - 2;

    SkScalar runFontTextSize = font.getSize();
    SkScalar fallbackTextSize = runFontTextSize;
    if (maxSourceGlyphDimension > maxAtlasDimension) {
        // Scale the text size down so the long side of all the glyphs will fit in the atlas.
        fallbackTextSize = SkScalarFloorToScalar(
                (maxAtlasDimension / maxSourceGlyphDimension) * runFontTextSize);
    }

    SkFont fallbackFont{font};
    fallbackFont.setSize(fallbackTextSize);

    // No sub-pixel needed. The transform to the screen will take care of sub-pixel positioning.
    fallbackFont.setSubpixel(false);

    // The scale factor to go from strike size to the source size for glyphs.
    SkScalar strikeToSourceRatio = runFontTextSize / fallbackTextSize;

    return SkStrikeSpec(fallbackFont, paint, surfaceProps, scalerContextFlags,
                        SkMatrix::I(), strikeToSourceRatio);
}

SkStrikeSpec SkStrikeSpec::MakeCanonicalized(const SkFont& font, const SkPaint* paint) {
    SkPaint canonicalizedPaint;
    if (paint != nullptr) {
        canonicalizedPaint = *paint;
    }

    const SkFont* canonicalizedFont = &font;
    SkTLazy<SkFont> pathFont;
    SkScalar strikeToSourceRatio = 1;
    if (ShouldDrawAsPath(canonicalizedPaint, font, SkMatrix::I())) {
        canonicalizedFont = pathFont.set(font);
        strikeToSourceRatio = pathFont->setupForAsPaths(nullptr);
        canonicalizedPaint.reset();
    }

    return SkStrikeSpec(*canonicalizedFont, canonicalizedPaint,
                        SkSurfaceProps(), kFakeGammaAndBoostContrast,
                        SkMatrix::I(), strikeToSourceRatio);
}

SkStrikeSpec SkStrikeSpec::MakeWithNoDevice(const SkFont& font, const SkPaint* paint) {
    SkPaint setupPaint;
    if (paint != nullptr) {
        setupPaint = *paint;
    }

    return SkStrikeSpec(font, setupPaint, SkSurfaceProps(), kFakeGammaAndBoostContrast,
                        SkMatrix::I(), 1);
}

SkStrikeSpec SkStrikeSpec::MakeDefault() {
    SkFont defaultFont;
    return MakeCanonicalized(defaultFont);
}

bool SkStrikeSpec::ShouldDrawAsPath(
        const SkPaint& paint, const SkFont& font, const SkMatrix& viewMatrix) {

    // hairline glyphs are fast enough so we don't need to cache them
    if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
        return true;
    }

    // we don't cache perspective
    if (viewMatrix.hasPerspective()) {
        return true;
    }

    SkMatrix textMatrix = SkFontPriv::MakeTextMatrix(font);
    textMatrix.postConcat(viewMatrix);

    // we have a self-imposed maximum, just to limit memory-usage
    SkScalar limit = std::min(SkGraphics::GetFontCachePointSizeLimit(), 1024);
    SkScalar maxSizeSquared = limit * limit;

    auto distance = [&textMatrix](int XIndex, int YIndex) {
        return textMatrix[XIndex] * textMatrix[XIndex] + textMatrix[YIndex] * textMatrix[YIndex];
    };

    return distance(SkMatrix::kMScaleX, SkMatrix::kMSkewY ) > maxSizeSquared
        || distance(SkMatrix::kMSkewX,  SkMatrix::kMScaleY) > maxSizeSquared;
}

SkString SkStrikeSpec::dump() const {
    return fAutoDescriptor.getDesc()->dumpRec();
}

SkStrikeSpec SkStrikeSpec::MakePDFVector(const SkTypeface& typeface, int* size) {
    SkFont font;
    font.setHinting(SkFontHinting::kNone);
    font.setEdging(SkFont::Edging::kAlias);
    font.setTypeface(sk_ref_sp(&typeface));
    int unitsPerEm = typeface.getUnitsPerEm();
    if (unitsPerEm <= 0) {
        unitsPerEm = 1024;
    }
    if (size) {
        *size = unitsPerEm;
    }
    font.setSize((SkScalar)unitsPerEm);

    return SkStrikeSpec(font, SkPaint(),
                        SkSurfaceProps(0, kUnknown_SkPixelGeometry), kFakeGammaAndBoostContrast,
                        SkMatrix::I(), 1);
}

#if SK_SUPPORT_GPU
std::tuple<SkStrikeSpec, SkScalar, SkScalar>
SkStrikeSpec::MakeSDFT(const SkFont& font, const SkPaint& paint,
                       const SkSurfaceProps& surfaceProps, const SkMatrix& deviceMatrix,
                       const GrSDFTControl& control) {
    SkPaint dfPaint{paint};
    dfPaint.setMaskFilter(GrSDFMaskFilter::Make());
    SkScalar strikeToSourceRatio;
    SkFont dfFont = control.getSDFFont(font, deviceMatrix, &strikeToSourceRatio);

    // Fake-gamma and subpixel antialiasing are applied in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;

    SkScalar minScale, maxScale;
    std::tie(minScale, maxScale) = control.computeSDFMinMaxScale(font.getSize(), deviceMatrix);

    SkStrikeSpec strikeSpec(dfFont, dfPaint, surfaceProps, flags,
                            SkMatrix::I(), strikeToSourceRatio);

    return std::make_tuple(std::move(strikeSpec), minScale, maxScale);
}

sk_sp<GrTextStrike> SkStrikeSpec::findOrCreateGrStrike(GrStrikeCache* cache) const {
    return cache->findOrCreateStrike(*fAutoDescriptor.getDesc());
}
#endif

SkStrikeSpec::SkStrikeSpec(const SkFont& font, const SkPaint& paint,
                           const SkSurfaceProps& surfaceProps,
                           SkScalerContextFlags scalerContextFlags,
                           const SkMatrix& deviceMatrix,
                           SkScalar strikeToSourceRatio)
    : fStrikeToSourceRatio(strikeToSourceRatio)
{
    SkScalerContextEffects effects;

    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            font, paint, surfaceProps, scalerContextFlags, deviceMatrix,
            &fAutoDescriptor, &effects);

    fMaskFilter = sk_ref_sp(effects.fMaskFilter);
    fPathEffect = sk_ref_sp(effects.fPathEffect);
    fTypeface = font.refTypefaceOrDefault();
}

SkScopedStrikeForGPU SkStrikeSpec::findOrCreateScopedStrike(SkStrikeForGPUCacheInterface* cache) const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return cache->findOrCreateScopedStrike(*fAutoDescriptor.getDesc(), effects, *fTypeface);
}

sk_sp<SkStrike> SkStrikeSpec::findOrCreateStrike(SkStrikeCache* cache) const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return cache->findOrCreateStrike(*fAutoDescriptor.getDesc(), effects, *fTypeface);
}

SkBulkGlyphMetrics::SkBulkGlyphMetrics(const SkStrikeSpec& spec)
    : fStrike{spec.findOrCreateStrike()} { }

SkSpan<const SkGlyph*> SkBulkGlyphMetrics::glyphs(SkSpan<const SkGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->metrics(glyphIDs, fGlyphs.get());
}

const SkGlyph* SkBulkGlyphMetrics::glyph(SkGlyphID glyphID) {
    return this->glyphs(SkSpan<const SkGlyphID>{&glyphID, 1})[0];
}

SkBulkGlyphMetricsAndPaths::SkBulkGlyphMetricsAndPaths(const SkStrikeSpec& spec)
    : fStrike{spec.findOrCreateStrike()} { }

SkBulkGlyphMetricsAndPaths::SkBulkGlyphMetricsAndPaths(sk_sp<SkStrike>&& strike)
        : fStrike{std::move(strike)} { }

SkSpan<const SkGlyph*> SkBulkGlyphMetricsAndPaths::glyphs(SkSpan<const SkGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->preparePaths(glyphIDs, fGlyphs.get());
}

const SkGlyph* SkBulkGlyphMetricsAndPaths::glyph(SkGlyphID glyphID) {
    return this->glyphs(SkSpan<const SkGlyphID>{&glyphID, 1})[0];
}

void SkBulkGlyphMetricsAndPaths::findIntercepts(
    const SkScalar* bounds, SkScalar scale, SkScalar xPos,
    const SkGlyph* glyph, SkScalar* array, int* count) {
    // TODO(herb): remove this abominable const_cast. Do the intercepts really need to be on the
    //  glyph?
    fStrike->findIntercepts(bounds, scale, xPos, const_cast<SkGlyph*>(glyph), array, count);
}

SkBulkGlyphMetricsAndImages::SkBulkGlyphMetricsAndImages(const SkStrikeSpec& spec)
        : fStrike{spec.findOrCreateStrike()} { }

SkBulkGlyphMetricsAndImages::SkBulkGlyphMetricsAndImages(sk_sp<SkStrike>&& strike)
        : fStrike{std::move(strike)} { }

SkSpan<const SkGlyph*> SkBulkGlyphMetricsAndImages::glyphs(SkSpan<const SkPackedGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->prepareImages(glyphIDs, fGlyphs.get());
}

const SkGlyph* SkBulkGlyphMetricsAndImages::glyph(SkPackedGlyphID packedID) {
    return this->glyphs(SkSpan<const SkPackedGlyphID>{&packedID, 1})[0];
}

const SkDescriptor& SkBulkGlyphMetricsAndImages::descriptor() const {
    return fStrike->getDescriptor();
}
