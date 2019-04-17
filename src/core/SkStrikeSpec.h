/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSTRIKESPEC_DEFINED
#define SKSTRIKESPEC_DEFINED

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

#include "SkDescriptor.h"
#include "SkStrikeInterface.h"

class SkStrikeSpecStorage {
public:
    static SkStrikeSpecStorage MakeMask(
            const SkFont& font, const SkPaint& paint,
            const SkSurfaceProps& surfaceProps, SkScalerContextFlags scalerContextFlags,
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

    static SkStrikeSpecStorage MakePath(
            const SkFont& font, const SkPaint& paint, const SkSurfaceProps& surfaceProps,
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

#if SK_SUPPORT_GPU
    static std::tuple<SkStrikeSpecStorage, SkScalar, SkScalar> MakeSDFT(
            const SkFont& font, const SkPaint& paint, const SkSurfaceProps& surfaceProps,
            const SkMatrix& deviceMatrix, const GrTextContext::Options& options) {
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
#endif

    SkScalar strikeToSourceRatio() const { return fStrikeToSourceRatio; }

    SkScopedStrike findOrCreateScopedStrike(SkStrikeCacheInterface* cache) {
        SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
        return cache->findOrCreateScopedStrike(*fAutoDescriptor.getDesc(), effects, *fTypeface);
    }

    const SkDescriptor& descriptor() const {
        return *fAutoDescriptor.getDesc();
    }

private:
    SkAutoDescriptor fAutoDescriptor;
    sk_sp<SkMaskFilter> fMaskFilter;
    sk_sp<SkPathEffect> fPathEffect;
    sk_sp<SkTypeface> fTypeface;
    SkScalar fStrikeToSourceRatio{1.0f};
};

#endif //SKSTRIKESPEC_DEFINED
