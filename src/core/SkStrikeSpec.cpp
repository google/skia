/*
 * Copyright 2019 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeSpec.h"

#include "src/core/SkDraw.h"
#include "src/core/SkStrikeCache.h"

SkStrikeSpecStorage SkStrikeSpecStorage::MakeMask(const SkFont& font, const SkPaint& paint,
                                                  const SkSurfaceProps& surfaceProps,
                                                  SkScalerContextFlags scalerContextFlags,
                                                  const SkMatrix& deviceMatrix) {
    SkStrikeSpecStorage storage;
    SkScalerContextEffects effects;

    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            font, paint, surfaceProps, scalerContextFlags, deviceMatrix,
            &storage.fAutoDescriptor, &effects);

    storage.fMaskFilter = sk_ref_sp(effects.fMaskFilter);
    storage.fPathEffect = sk_ref_sp(effects.fPathEffect);
    storage.fTypeface = font.refTypefaceOrDefault();

    return storage;
}

SkStrikeSpecStorage SkStrikeSpecStorage::MakePath(const SkFont& font, const SkPaint& paint,
                                                  const SkSurfaceProps& surfaceProps,
                                                  SkScalerContextFlags scalerContextFlags) {
    SkStrikeSpecStorage storage;

    // setup our std runPaint, in hopes of getting hits in the cache
    SkPaint pathPaint{paint};
    SkFont pathFont{font};

    // The factor to get from the size stored in the strike to the size needed for
    // the source.
    storage.fStrikeToSourceRatio = pathFont.setupForAsPaths(&pathPaint);

    // The sub-pixel position will always happen when transforming to the screen.
    pathFont.setSubpixel(false);

    SkScalerContextEffects effects;
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(pathFont,
                                                          pathPaint,
                                                          surfaceProps,
                                                          scalerContextFlags,
                                                          SkMatrix::I(),
                                                          &storage.fAutoDescriptor,
                                                          &effects);

    storage.fMaskFilter = sk_ref_sp(effects.fMaskFilter);
    storage.fPathEffect = sk_ref_sp(effects.fPathEffect);
    storage.fTypeface = pathFont.refTypefaceOrDefault();

    return storage;
}

SkStrikeSpecStorage SkStrikeSpecStorage::MakeCanonicalized(
        const SkFont& font, const SkPaint* paint) {
    SkStrikeSpecStorage storage;

    SkPaint canonicalizedPaint;
    if (paint != nullptr) {
        canonicalizedPaint = *paint;
    }

    const SkFont* canonicalizedFont = &font;
    SkTLazy<SkFont> pathFont;
    if (
#ifdef SK_IGNORE_LINEAR_METRICS_FIX
            font.isLinearMetrics() ||
#endif
            SkDraw::ShouldDrawTextAsPaths(font, canonicalizedPaint, SkMatrix::I())) {
        canonicalizedFont = pathFont.set(font);
        storage.fStrikeToSourceRatio = pathFont->setupForAsPaths(nullptr);
        canonicalizedPaint.reset();
    }

    SkScalerContextEffects effects;
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            *canonicalizedFont,
            canonicalizedPaint,
            SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType),
            kFakeGammaAndBoostContrast,
            SkMatrix::I(),
            &storage.fAutoDescriptor,
            &effects);

    storage.fMaskFilter = sk_ref_sp(effects.fMaskFilter);
    storage.fPathEffect = sk_ref_sp(effects.fPathEffect);
    storage.fTypeface = canonicalizedFont->refTypefaceOrDefault();

    return storage;
}

SkStrikeSpecStorage SkStrikeSpecStorage::MakeDefault() {
    SkFont defaultFont;
    return MakeCanonicalized(defaultFont);
}

#if SK_SUPPORT_GPU
std::tuple<SkStrikeSpecStorage, SkScalar, SkScalar>
SkStrikeSpecStorage::MakeSDFT(const SkFont& font, const SkPaint& paint,
                              const SkSurfaceProps& surfaceProps, const SkMatrix& deviceMatrix,
                              const GrTextContext::Options& options) {
    SkStrikeSpecStorage storage;

    SkPaint dfPaint = GrTextContext::InitDistanceFieldPaint(paint);
    SkFont dfFont = GrTextContext::InitDistanceFieldFont(
            font, deviceMatrix, options, &storage.fStrikeToSourceRatio);

    // Fake-gamma and subpixel antialiasing are applied in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;

    SkScalar minScale, maxScale;
    std::tie(minScale, maxScale) = GrTextContext::InitDistanceFieldMinMaxScale(
            font.getSize(), deviceMatrix, options);

    SkScalerContextEffects effects;
    SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            dfFont, dfPaint, surfaceProps, flags,
            SkMatrix::I(), &storage.fAutoDescriptor, &effects);

    storage.fMaskFilter = sk_ref_sp(effects.fMaskFilter);
    storage.fPathEffect = sk_ref_sp(effects.fPathEffect);
    storage.fTypeface = dfFont.refTypefaceOrDefault();

    return std::tie(storage, minScale, maxScale);
}

sk_sp<GrTextStrike> SkStrikeSpecStorage::findOrCreateGrStrike(GrStrikeCache* cache) const {
    return cache->getStrike(*fAutoDescriptor.getDesc());
}
#endif

SkScopedStrike SkStrikeSpecStorage::findOrCreateScopedStrike(SkStrikeCacheInterface* cache) const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return cache->findOrCreateScopedStrike(*fAutoDescriptor.getDesc(), effects, *fTypeface);
}

SkExclusiveStrikePtr SkStrikeSpecStorage::findOrCreateExclusiveStrike(SkStrikeCache* cache) const {
    SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
    return cache->findOrCreateStrikeExclusive(*fAutoDescriptor.getDesc(), effects, *fTypeface);
}
