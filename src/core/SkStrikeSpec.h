/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeSpec_DEFINED
#define SkStrikeSpec_DEFINED

#include "src/core/SkDescriptor.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeInterface.h"

#ifdef SK_SUPPORT_GPU
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextContext.h"
#endif

class SkFont;
class SkPaint;
class SkStrikeCache;
class SkSurfaceProps;

class SkStrikeSpecStorage {
public:
    static SkStrikeSpecStorage MakeMask(
            const SkFont& font, const SkPaint& paint,
            const SkSurfaceProps& surfaceProps, SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    static SkStrikeSpecStorage MakePath(
            const SkFont& font, const SkPaint& paint, const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags);

    static SkStrikeSpecStorage MakeCanonicalized(
            const SkFont& font, const SkPaint* paint = nullptr);

    static SkStrikeSpecStorage MakeDefault();

#if SK_SUPPORT_GPU
    static std::tuple<SkStrikeSpecStorage, SkScalar, SkScalar> MakeSDFT(
            const SkFont& font, const SkPaint& paint, const SkSurfaceProps& surfaceProps,
            const SkMatrix& deviceMatrix, const GrTextContext::Options& options);

    sk_sp<GrTextStrike> findOrCreateGrStrike(GrStrikeCache* cache) const;
#endif

    SkScopedStrike findOrCreateScopedStrike(SkStrikeCacheInterface* cache) const;

    SkExclusiveStrikePtr findOrCreateExclusiveStrike(
            SkStrikeCache* cache = SkStrikeCache::GlobalStrikeCache()) const;

    SkScalar strikeToSourceRatio() const { return fStrikeToSourceRatio; }
    const SkDescriptor& descriptor() const { return *fAutoDescriptor.getDesc(); }

private:
    SkAutoDescriptor fAutoDescriptor;
    sk_sp<SkMaskFilter> fMaskFilter;
    sk_sp<SkPathEffect> fPathEffect;
    sk_sp<SkTypeface> fTypeface;
    SkScalar fStrikeToSourceRatio{1.0f};
};

#endif  // SkStrikeSpec_DEFINED
