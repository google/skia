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
#include "src/core/SkStrikeForGPU.h"

#if SK_SUPPORT_GPU
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextContext.h"
#endif

class SkFont;
class SkPaint;
class SkStrikeCache;
class SkSurfaceProps;

class SkStrikeSpec {
public:
    // Create a strike spec for mask style cache entries.
    static SkStrikeSpec MakeMask(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    // Create a strike spec for path style cache entries.
    static SkStrikeSpec MakePath(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags);

    static SkStrikeSpec MakeSourceFallback(const SkFont& font,
                                                  const SkPaint& paint,
                                                  const SkSurfaceProps& surfaceProps,
                                                  SkScalerContextFlags scalerContextFlags,
                                                  SkScalar maxSourceGlyphDimension);

    // Create a canonical strike spec for device-less measurements.
    static SkStrikeSpec MakeCanonicalized(
            const SkFont& font, const SkPaint* paint = nullptr);

    // Create a strike spec without a device, and does not switch over to path for large sizes.
    // This means that strikeToSourceRatio() is always 1.
    static SkStrikeSpec MakeWithNoDevice(const SkFont& font, const SkPaint* paint = nullptr);

    // Make a canonical strike spec for device-less measurements using default typeface and size.
    static SkStrikeSpec MakeDefault();

    // Make a strike spec for PDF Vector strikes
    static SkStrikeSpec MakePDFVector(const SkTypeface& typeface, int* size);

#if SK_SUPPORT_GPU
    // Create a strike spec for scaled distance field text.
    static std::tuple<SkStrikeSpec, SkScalar, SkScalar> MakeSDFT(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            const SkMatrix& deviceMatrix,
            const GrTextContext::Options& options);

    sk_sp<GrTextStrike> findOrCreateGrStrike(GrStrikeCache* cache) const;
#endif

    SkScopedStrikeForGPU findOrCreateScopedStrike(SkStrikeForGPUCacheInterface* cache) const;

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

class SkBulkGlyphMetrics {
public:
    explicit SkBulkGlyphMetrics(const SkStrikeSpec& spec);
    SkSpan<const SkGlyph*> glyphs(SkSpan<const SkGlyphID> glyphIDs);

private:
    static constexpr int kTypicalGlyphCount = 20;
    SkAutoSTArray<kTypicalGlyphCount, const SkGlyph*> fGlyphs;
    SkExclusiveStrikePtr fStrike;
};

class SkBulkGlyphMetricsAndPaths {
public:
    explicit SkBulkGlyphMetricsAndPaths(const SkStrikeSpec& spec);
    SkSpan<const SkGlyph*> glyphs(SkSpan<const SkGlyphID> glyphIDs);

private:
    static constexpr int kTypicalGlyphCount = 20;
    SkAutoSTArray<kTypicalGlyphCount, const SkGlyph*> fGlyphs;
    SkExclusiveStrikePtr fStrike;
};

class SkBulkGlyphMetricsAndImages {
public:
    explicit SkBulkGlyphMetricsAndImages(const SkStrikeSpec& spec);
    SkSpan<const SkGlyph*> glyphs(SkSpan<const SkPackedGlyphID> glyphIDs);

private:
    static constexpr int kTypicalGlyphCount = 64;
    SkAutoSTArray<kTypicalGlyphCount, const SkGlyph*> fGlyphs;
    SkExclusiveStrikePtr fStrike;
};

#endif  // SkStrikeSpec_DEFINED
