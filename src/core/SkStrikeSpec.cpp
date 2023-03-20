/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeSpec.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkDraw.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"

#if defined(SK_GANESH) || defined(SK_GRAPHITE)
#include "src/text/gpu/SDFMaskFilter.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/StrikeCache.h"
#endif

SkStrikeSpec::SkStrikeSpec(const SkDescriptor& descriptor, sk_sp<SkTypeface> typeface)
    : fAutoDescriptor{descriptor}
    , fTypeface{std::move(typeface)} {}

SkStrikeSpec::SkStrikeSpec(const SkStrikeSpec&) = default;
SkStrikeSpec::SkStrikeSpec(SkStrikeSpec&&) = default;
SkStrikeSpec::~SkStrikeSpec() = default;

SkStrikeSpec SkStrikeSpec::MakeMask(const SkFont& font, const SkPaint& paint,
                                    const SkSurfaceProps& surfaceProps,
                                    SkScalerContextFlags scalerContextFlags,
                                    const SkMatrix& deviceMatrix) {

    return SkStrikeSpec(font, paint, surfaceProps, scalerContextFlags, deviceMatrix);
}

SkStrikeSpec SkStrikeSpec::MakeTransformMask(const SkFont& font,
                                             const SkPaint& paint,
                                             const SkSurfaceProps& surfaceProps,
                                             SkScalerContextFlags scalerContextFlags,
                                             const SkMatrix& deviceMatrix) {
    SkFont sourceFont{font};
    sourceFont.setSubpixel(false);
    return SkStrikeSpec(sourceFont, paint, surfaceProps, scalerContextFlags, deviceMatrix);
}

std::tuple<SkStrikeSpec, SkScalar> SkStrikeSpec::MakePath(
        const SkFont& font, const SkPaint& paint,
        const SkSurfaceProps& surfaceProps,
        SkScalerContextFlags scalerContextFlags) {

    // setup our std runPaint, in hopes of getting hits in the cache
    SkPaint pathPaint{paint};
    SkFont pathFont{font};

    // The sub-pixel position will always happen when transforming to the screen.
    pathFont.setSubpixel(false);

    // The factor to get from the size stored in the strike to the size needed for
    // the source.
    SkScalar strikeToSourceScale = pathFont.setupForAsPaths(&pathPaint);

    return {SkStrikeSpec(pathFont, pathPaint, surfaceProps, scalerContextFlags, SkMatrix::I()),
            strikeToSourceScale};
}

std::tuple<SkStrikeSpec, SkScalar> SkStrikeSpec::MakeCanonicalized(
        const SkFont& font, const SkPaint* paint) {
    SkPaint canonicalizedPaint;
    if (paint != nullptr) {
        canonicalizedPaint = *paint;
    }

    const SkFont* canonicalizedFont = &font;
    SkTLazy<SkFont> pathFont;
    SkScalar strikeToSourceScale = 1;
    if (ShouldDrawAsPath(canonicalizedPaint, font, SkMatrix::I())) {
        canonicalizedFont = pathFont.set(font);
        strikeToSourceScale = pathFont->setupForAsPaths(nullptr);
        canonicalizedPaint.reset();
    }

    return {SkStrikeSpec(*canonicalizedFont, canonicalizedPaint, SkSurfaceProps(),
                         SkScalerContextFlags::kFakeGammaAndBoostContrast, SkMatrix::I()),
            strikeToSourceScale};
}

SkStrikeSpec SkStrikeSpec::MakeWithNoDevice(const SkFont& font, const SkPaint* paint) {
    SkPaint setupPaint;
    if (paint != nullptr) {
        setupPaint = *paint;
    }

    return SkStrikeSpec(font, setupPaint, SkSurfaceProps(),
                        SkScalerContextFlags::kFakeGammaAndBoostContrast, SkMatrix::I());
}

bool SkStrikeSpec::ShouldDrawAsPath(
        const SkPaint& paint, const SkFont& font, const SkMatrix& viewMatrix) {

    // hairline glyphs are fast enough, so we don't need to cache them
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
    constexpr SkScalar memoryLimit = 256;
    constexpr SkScalar maxSizeSquared = memoryLimit * memoryLimit;

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

    return SkStrikeSpec(font,
                        SkPaint(),
                        SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                        SkScalerContextFlags::kFakeGammaAndBoostContrast,
                        SkMatrix::I());
}

#if (defined(SK_GANESH) || defined(SK_GRAPHITE)) && !defined(SK_DISABLE_SDF_TEXT)
std::tuple<SkStrikeSpec, SkScalar, sktext::gpu::SDFTMatrixRange>
SkStrikeSpec::MakeSDFT(const SkFont& font, const SkPaint& paint,
                       const SkSurfaceProps& surfaceProps, const SkMatrix& deviceMatrix,
                       const SkPoint& textLocation, const sktext::gpu::SDFTControl& control) {
    // Add filter to the paint which creates the SDFT data for A8 masks.
    SkPaint dfPaint{paint};
    dfPaint.setMaskFilter(sktext::gpu::SDFMaskFilter::Make());

    auto [dfFont, strikeToSourceScale, matrixRange] = control.getSDFFont(font, deviceMatrix,
                                                                         textLocation);

    // Adjust the stroke width by the scale factor for drawing the SDFT.
    dfPaint.setStrokeWidth(paint.getStrokeWidth() / strikeToSourceScale);

    // Check for dashing and adjust the intervals.
    if (SkPathEffect* pathEffect = paint.getPathEffect(); pathEffect != nullptr) {
        SkPathEffect::DashInfo dashInfo;
        if (pathEffect->asADash(&dashInfo) == SkPathEffect::kDash_DashType) {
            if (dashInfo.fCount > 0) {
                // Allocate the intervals.
                std::vector<SkScalar> scaledIntervals(dashInfo.fCount);
                dashInfo.fIntervals = scaledIntervals.data();
                // Call again to get the interval data.
                (void)pathEffect->asADash(&dashInfo);
                for (SkScalar& interval : scaledIntervals) {
                    interval /= strikeToSourceScale;
                }
                auto scaledDashes = SkDashPathEffect::Make(scaledIntervals.data(),
                                                           scaledIntervals.size(),
                                                           dashInfo.fPhase / strikeToSourceScale);
                dfPaint.setPathEffect(scaledDashes);
            }
        }
    }

    // Fake-gamma and subpixel antialiasing are applied in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;
    SkStrikeSpec strikeSpec(dfFont, dfPaint, surfaceProps, flags, SkMatrix::I());

    return std::make_tuple(std::move(strikeSpec), strikeToSourceScale, matrixRange);
}
#endif

SkStrikeSpec::SkStrikeSpec(const SkFont& font, const SkPaint& paint,
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

sk_sp<sktext::StrikeForGPU> SkStrikeSpec::findOrCreateScopedStrike(
        sktext::StrikeForGPUCacheInterface* cache) const {
    return cache->findOrCreateScopedStrike(*this);
}

sk_sp<SkStrike> SkStrikeSpec::findOrCreateStrike() const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return SkStrikeCache::GlobalStrikeCache()->findOrCreateStrike(*this);
}

sk_sp<SkStrike> SkStrikeSpec::findOrCreateStrike(SkStrikeCache* cache) const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return cache->findOrCreateStrike(*this);
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

SkBulkGlyphMetricsAndPaths::~SkBulkGlyphMetricsAndPaths() = default;

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

SkBulkGlyphMetricsAndDrawables::SkBulkGlyphMetricsAndDrawables(const SkStrikeSpec& spec)
        : fStrike{spec.findOrCreateStrike()} { }

SkBulkGlyphMetricsAndDrawables::SkBulkGlyphMetricsAndDrawables(sk_sp<SkStrike>&& strike)
        : fStrike{std::move(strike)} { }

SkBulkGlyphMetricsAndDrawables::~SkBulkGlyphMetricsAndDrawables() = default;

SkSpan<const SkGlyph*> SkBulkGlyphMetricsAndDrawables::glyphs(SkSpan<const SkGlyphID> glyphIDs) {
    fGlyphs.reset(glyphIDs.size());
    return fStrike->prepareDrawables(glyphIDs, fGlyphs.get());
}

const SkGlyph* SkBulkGlyphMetricsAndDrawables::glyph(SkGlyphID glyphID) {
    return this->glyphs(SkSpan<const SkGlyphID>{&glyphID, 1})[0];
}

SkBulkGlyphMetricsAndImages::SkBulkGlyphMetricsAndImages(const SkStrikeSpec& spec)
        : fStrike{spec.findOrCreateStrike()} { }

SkBulkGlyphMetricsAndImages::SkBulkGlyphMetricsAndImages(sk_sp<SkStrike>&& strike)
        : fStrike{std::move(strike)} { }

SkBulkGlyphMetricsAndImages::~SkBulkGlyphMetricsAndImages() = default;

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
