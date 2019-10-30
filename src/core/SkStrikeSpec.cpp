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

SkStrikeSpec SkStrikeSpec::MakeMask(const SkFont& font, const SkPaint& paint,
                                    const SkSurfaceProps& surfaceProps,
                                    SkScalerContextFlags scalerContextFlags,
                                    const SkMatrix& deviceMatrix) {
    SkStrikeSpec storage;

    storage.commonSetup(font, paint, surfaceProps, scalerContextFlags, deviceMatrix);

    return storage;
}

SkStrikeSpec SkStrikeSpec::MakePath(const SkFont& font, const SkPaint& paint,
                                    const SkSurfaceProps& surfaceProps,
                                    SkScalerContextFlags scalerContextFlags) {
    SkStrikeSpec storage;

    // setup our std runPaint, in hopes of getting hits in the cache
    SkPaint pathPaint{paint};
    SkFont pathFont{font};

    // The factor to get from the size stored in the strike to the size needed for
    // the source.
    storage.fStrikeToSourceRatio = pathFont.setupForAsPaths(&pathPaint);

    // The sub-pixel position will always happen when transforming to the screen.
    pathFont.setSubpixel(false);

    storage.commonSetup(pathFont, pathPaint, surfaceProps, scalerContextFlags, SkMatrix::I());

    return storage;
}

SkStrikeSpec SkStrikeSpec::MakeSourceFallback(
        const SkFont& font,
        const SkPaint& paint,
        const SkSurfaceProps& surfaceProps,
        SkScalerContextFlags scalerContextFlags,
        SkScalar maxSourceGlyphDimension) {
    SkStrikeSpec storage;

    // Subtract 2 to account for the bilerp pad around the glyph
    SkScalar maxAtlasDimension = SkStrikeCommon::kSkSideTooBigForAtlas - 2;

    SkScalar runFontTextSize = font.getSize();

    // Scale the text size down so the long side of all the glyphs will fit in the atlas.
    SkScalar fallbackTextSize = SkScalarFloorToScalar(
            (maxAtlasDimension / maxSourceGlyphDimension) * runFontTextSize);

    SkFont fallbackFont{font};
    fallbackFont.setSize(fallbackTextSize);

    // No sub-pixel needed. The transform to the screen will take care of sub-pixel positioning.
    fallbackFont.setSubpixel(false);

    // The scale factor to go from strike size to the source size for glyphs.
    storage.fStrikeToSourceRatio = runFontTextSize / fallbackTextSize;

    storage.commonSetup(fallbackFont, paint, surfaceProps, scalerContextFlags, SkMatrix::I());

    return storage;
}

SkStrikeSpec SkStrikeSpec::MakeCanonicalized(const SkFont& font, const SkPaint* paint) {
    SkStrikeSpec storage;

    SkPaint canonicalizedPaint;
    if (paint != nullptr) {
        canonicalizedPaint = *paint;
    }

    const SkFont* canonicalizedFont = &font;
    SkTLazy<SkFont> pathFont;
    if (ShouldDrawAsPath(canonicalizedPaint, font, SkMatrix::I())) {
        canonicalizedFont = pathFont.set(font);
        storage.fStrikeToSourceRatio = pathFont->setupForAsPaths(nullptr);
        canonicalizedPaint.reset();
    }

    storage.commonSetup(*canonicalizedFont,
                        canonicalizedPaint,
                        SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType),
                        kFakeGammaAndBoostContrast,
                        SkMatrix::I());
    return storage;
}

SkStrikeSpec SkStrikeSpec::MakeWithNoDevice(const SkFont& font, const SkPaint* paint) {
    SkStrikeSpec storage;

    SkPaint setupPaint;
    if (paint != nullptr) {
        setupPaint = *paint;
    }

    storage.commonSetup(font,
                        setupPaint,
                        SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType),
                        kFakeGammaAndBoostContrast,
                        SkMatrix::I());

    return storage;

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

    // we have a self-imposed maximum, just for memory-usage sanity
    SkScalar limit = SkMinScalar(SkGraphics::GetFontCachePointSizeLimit(), 1024);
    SkScalar maxSizeSquared = limit * limit;

    auto distance = [&textMatrix](int XIndex, int YIndex) {
        return textMatrix[XIndex] * textMatrix[XIndex] + textMatrix[YIndex] * textMatrix[YIndex];
    };

    return distance(SkMatrix::kMScaleX, SkMatrix::kMSkewY ) > maxSizeSquared
        || distance(SkMatrix::kMSkewX,  SkMatrix::kMScaleY) > maxSizeSquared;
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

    SkStrikeSpec storage;
    storage.commonSetup(font,
                        SkPaint(),
                        SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                        kFakeGammaAndBoostContrast,
                        SkMatrix::I());

    return storage;
}

#if SK_SUPPORT_GPU
std::tuple<SkStrikeSpec, SkScalar, SkScalar>
SkStrikeSpec::MakeSDFT(const SkFont& font, const SkPaint& paint,
                       const SkSurfaceProps& surfaceProps, const SkMatrix& deviceMatrix,
                       const GrTextContext::Options& options) {
    SkStrikeSpec storage;

    SkPaint dfPaint = GrTextContext::InitDistanceFieldPaint(paint);
    SkFont dfFont = GrTextContext::InitDistanceFieldFont(
            font, deviceMatrix, options, &storage.fStrikeToSourceRatio);

    // Fake-gamma and subpixel antialiasing are applied in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;

    SkScalar minScale, maxScale;
    std::tie(minScale, maxScale) = GrTextContext::InitDistanceFieldMinMaxScale(
            font.getSize(), deviceMatrix, options);

    storage.commonSetup(dfFont, dfPaint, surfaceProps, flags, SkMatrix::I());

    return std::tie(storage, minScale, maxScale);
}

sk_sp<GrTextStrike> SkStrikeSpec::findOrCreateGrStrike(GrStrikeCache* cache) const {
    return cache->getStrike(*fAutoDescriptor.getDesc());
}
#endif

void SkStrikeSpec::commonSetup(const SkFont& font, const SkPaint& paint,
                               const SkSurfaceProps& surfaceProps,
                               SkScalerContextFlags scalerContextFlags,
                               const SkMatrix& deviceMatrix) {
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

SkExclusiveStrikePtr SkStrikeSpec::findOrCreateExclusiveStrike(SkStrikeCache* cache) const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return cache->findOrCreateStrikeExclusive(*fAutoDescriptor.getDesc(), effects, *fTypeface);
}

SkBulkGlyphMetrics::SkBulkGlyphMetrics(const SkStrikeSpec& spec)
    : fStrike{spec.findOrCreateExclusiveStrike()} { }

SkSpan<const SkGlyph*> SkBulkGlyphMetrics::glyphs(SkSpan<const SkGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->metrics(glyphIDs, fGlyphs.get());
}

SkBulkGlyphMetricsAndPaths::SkBulkGlyphMetricsAndPaths(const SkStrikeSpec& spec)
    : fStrike{spec.findOrCreateExclusiveStrike()} { }

SkSpan<const SkGlyph*> SkBulkGlyphMetricsAndPaths::glyphs(SkSpan<const SkGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->preparePaths(glyphIDs, fGlyphs.get());
}

SkBulkGlyphMetricsAndImages::SkBulkGlyphMetricsAndImages(const SkStrikeSpec& spec)
        : fStrike{spec.findOrCreateExclusiveStrike()} { }

SkSpan<const SkGlyph*> SkBulkGlyphMetricsAndImages::glyphs(SkSpan<const SkPackedGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->prepareImages(glyphIDs, fGlyphs.get());
}
