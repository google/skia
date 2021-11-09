/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeSpec_DEFINED
#define SkStrikeSpec_DEFINED

#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkStrikeForGPU.h"

#include <tuple>

#if SK_SUPPORT_GPU
#include "src/gpu/text/GrSDFTControl.h"
class GrStrikeCache;
class GrTextStrike;
#endif

class SkFont;
class SkPaint;
class SkStrikeCache;
class SkSurfaceProps;

class SkStrikeSpec {
public:
    SkStrikeSpec(const SkDescriptor& descriptor, sk_sp<SkTypeface> typeface);
    SkStrikeSpec(const SkStrikeSpec&);
    SkStrikeSpec& operator=(const SkStrikeSpec&) = delete;

    SkStrikeSpec(SkStrikeSpec&&);
    SkStrikeSpec& operator=(SkStrikeSpec&&) = delete;

    ~SkStrikeSpec();

    // Create a strike spec for mask style cache entries.
    static SkStrikeSpec MakeMask(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    // Create a strike spec for path style cache entries.
    static std::tuple<SkStrikeSpec, SkScalar> MakePath(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags);

    static std::tuple<SkStrikeSpec, SkScalar> MakeSourceFallback(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            SkScalar maxSourceGlyphDimension);

    // Create a canonical strike spec for device-less measurements.
    static std::tuple<SkStrikeSpec, SkScalar> MakeCanonicalized(
            const SkFont& font, const SkPaint* paint = nullptr);

    // Create a strike spec without a device, and does not switch over to path for large sizes.
    static SkStrikeSpec MakeWithNoDevice(const SkFont& font, const SkPaint* paint = nullptr);

    // Make a strike spec for PDF Vector strikes
    static SkStrikeSpec MakePDFVector(const SkTypeface& typeface, int* size);

#if SK_SUPPORT_GPU
    // Create a strike spec for scaled distance field text.
    static std::tuple<SkStrikeSpec, SkScalar, SkScalar, SkScalar> MakeSDFT(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            const SkMatrix& deviceMatrix,
            const GrSDFTControl& control);

    sk_sp<GrTextStrike> findOrCreateGrStrike(GrStrikeCache* cache) const;
#endif

    SkScopedStrikeForGPU findOrCreateScopedStrike(SkStrikeForGPUCacheInterface* cache) const;

    sk_sp<SkStrike> findOrCreateStrike() const;

    sk_sp<SkStrike> findOrCreateStrike(SkStrikeCache* cache) const;

    std::unique_ptr<SkScalerContext> createScalerContext() const {
        SkScalerContextEffects effects{fPathEffect.get(), fMaskFilter.get()};
        return fTypeface->createScalerContext(effects, fAutoDescriptor.getDesc());
    }

    const SkDescriptor& descriptor() const { return *fAutoDescriptor.getDesc(); }
    const SkTypeface& typeface() const { return *fTypeface; }
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkFont& font, const SkMatrix& matrix);
    SkString dump() const;

private:
    SkStrikeSpec(
            const SkFont& font,
            const SkPaint& paint,
            const SkSurfaceProps& surfaceProps,
            SkScalerContextFlags scalerContextFlags,
            const SkMatrix& deviceMatrix);

    SkAutoDescriptor fAutoDescriptor;
    sk_sp<SkMaskFilter> fMaskFilter{nullptr};
    sk_sp<SkPathEffect> fPathEffect{nullptr};
    sk_sp<SkTypeface> fTypeface;
};

class SkBulkGlyphMetrics {
public:
    explicit SkBulkGlyphMetrics(const SkStrikeSpec& spec);
    SkSpan<const SkGlyph*> glyphs(SkSpan<const SkGlyphID> glyphIDs);
    const SkGlyph* glyph(SkGlyphID glyphID);

private:
    inline static constexpr int kTypicalGlyphCount = 20;
    SkAutoSTArray<kTypicalGlyphCount, const SkGlyph*> fGlyphs;
    sk_sp<SkStrike> fStrike;
};

class SkBulkGlyphMetricsAndPaths {
public:
    explicit SkBulkGlyphMetricsAndPaths(const SkStrikeSpec& spec);
    explicit SkBulkGlyphMetricsAndPaths(sk_sp<SkStrike>&& strike);
    ~SkBulkGlyphMetricsAndPaths();
    SkSpan<const SkGlyph*> glyphs(SkSpan<const SkGlyphID> glyphIDs);
    const SkGlyph* glyph(SkGlyphID glyphID);
    void findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                        const SkGlyph* glyph, SkScalar* array, int* count);

private:
    inline static constexpr int kTypicalGlyphCount = 20;
    SkAutoSTArray<kTypicalGlyphCount, const SkGlyph*> fGlyphs;
    sk_sp<SkStrike> fStrike;
};

class SkBulkGlyphMetricsAndImages {
public:
    explicit SkBulkGlyphMetricsAndImages(const SkStrikeSpec& spec);
    explicit SkBulkGlyphMetricsAndImages(sk_sp<SkStrike>&& strike);
    ~SkBulkGlyphMetricsAndImages();
    SkSpan<const SkGlyph*> glyphs(SkSpan<const SkPackedGlyphID> packedIDs);
    const SkGlyph* glyph(SkPackedGlyphID packedID);
    const SkDescriptor& descriptor() const;

private:
    inline static constexpr int kTypicalGlyphCount = 64;
    SkAutoSTArray<kTypicalGlyphCount, const SkGlyph*> fGlyphs;
    sk_sp<SkStrike> fStrike;
};

#endif  // SkStrikeSpec_DEFINED
