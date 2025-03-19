/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_proxy_DEFINED
#define SkTypeface_proxy_DEFINED
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "src/core/SkScalerContext.h"

#include <string.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

class SkTypeface_proxy;
class SkScalerContext_proxy : public SkScalerContext {
public:
    SkScalerContext_proxy(std::unique_ptr<SkScalerContext> realScalerContext,
                          SkTypeface_proxy& proxyTypeface,
                          const SkScalerContextEffects& effects,
                          const SkDescriptor* desc);

    ~SkScalerContext_proxy() override = default;
protected:
    SkScalerContext::GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc* alloc) override;
    void generateImage(const SkGlyph& glyph, void* imageBuffer) override;
    bool generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) override;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override;
    void generateFontMetrics(SkFontMetrics* metrics) override;
private:
    std::unique_ptr<SkScalerContext> fRealScalerContext;
};

class SkTypeface_proxy : public SkTypeface {
public:
    SkTypeface_proxy(sk_sp<SkTypeface> realTypeface,
                     const SkFontStyle& style, bool isFixedPitch = false)
        : SkTypeface(style, isFixedPitch)
        , fRealTypeface(std::move(realTypeface)) { SkASSERT_RELEASE(fRealTypeface); }

protected:
    int onGetUPEM() const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    bool onGlyphMaskNeedsCurrentColor() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    SkFontStyle onGetFontStyle() const override;
    bool onGetFixedPitch() const override;
    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString* postScriptName) const override;
    int onGetResourceName(SkString* resourceName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;
    std::unique_ptr<SkScalerContext> onCreateScalerContext(
            const SkScalerContextEffects& effects, const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec* rec) const override;
    void onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const override;
    void getGlyphToUnicodeMap(SkUnichar* unichar) const override;
    void getPostScriptGlyphNames(SkString* glyphNames) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    void* onGetCTFontRef() const override;
    bool onGetKerningPairAdjustments(const SkGlyphID glyphs[],
                                     int count,
                                     int32_t adjustments[]) const override;
private:
    sk_sp<SkTypeface> fRealTypeface;
};

#endif // SkTypeface_proxy_DEFINED
