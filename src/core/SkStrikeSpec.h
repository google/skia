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

#if SK_SUPPORT_GPU
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextContext.h"
#endif

class SkFont;
class SkPaint;
class SkStrikeCache;
class SkSurfaceProps;

// TODO: rename to SkStrikeSpec when the current SkStrikeSpec is remove from the code.
class SkStrikeSpecStorage {
public:
    // Create a strike spec for mask style cache entries.
    static SkStrikeSpecStorage MakeMask(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    // Create a strike spec for path style cache entries.
    static SkStrikeSpecStorage MakePath(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags);

    static SkStrikeSpecStorage MakeSourceFallback(const SkFont& font,
                                                  const SkPaint& paint,
                                                  const SkSurfaceProps& surfaceProps,
                                                  SkScalerContextFlags scalerContextFlags,
                                                  SkScalar maxSourceGlyphDimension);

    // Create a canonical strike spec for device-less measurements.
    static SkStrikeSpecStorage MakeCanonicalized(
            const SkFont& font, const SkPaint* paint = nullptr);

    // Make a canonical strike spec for device-less measurements using default typeface and size.
    static SkStrikeSpecStorage MakeDefault();

    // Make a strike spec for PDF Vector strikes
    static SkStrikeSpecStorage MakePDFVector(const SkTypeface& typeface, int* size);

#if SK_SUPPORT_GPU
    // Create a strike spec for scaled distance field text.
    static std::tuple<SkStrikeSpecStorage, SkScalar, SkScalar> MakeSDFT(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            const SkMatrix& deviceMatrix,
            const GrTextContext::Options& options);

    sk_sp<GrTextStrike> findOrCreateGrStrike(GrStrikeCache* cache) const;
#endif

    SkScopedStrike findOrCreateScopedStrike(SkStrikeCacheInterface* cache) const;

    SkExclusiveStrikePtr findOrCreateExclusiveStrike(
            SkStrikeCache* cache = SkStrikeCache::GlobalStrikeCache()) const;

    SkScalar strikeToSourceRatio() const { return fStrikeToSourceRatio; }
    const SkDescriptor& descriptor() const { return *fAutoDescriptor.getDesc(); }
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkFont& font, const SkMatrix& matrix);

private:
    void commonSetup(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    SkAutoDescriptor fAutoDescriptor;
    sk_sp<SkMaskFilter> fMaskFilter;
    sk_sp<SkPathEffect> fPathEffect;
    sk_sp<SkTypeface> fTypeface;
    SkScalar fStrikeToSourceRatio{1.0f};
};

#endif  // SkStrikeSpec_DEFINED
