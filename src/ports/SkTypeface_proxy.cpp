/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/ports/SkTypeface_proxy.h"

#include <memory>
#include <utility>

int SkTypeface_proxy::onGetUPEM() const { return fRealTypeface->getUnitsPerEm(); }

std::unique_ptr<SkStreamAsset> SkTypeface_proxy::onOpenStream(int* ttcIndex) const {
    return fRealTypeface->onOpenStream(ttcIndex);
}

sk_sp<SkTypeface> SkTypeface_proxy::onMakeClone(const SkFontArguments& args) const {
    return fRealTypeface->onMakeClone(args);
}

bool SkTypeface_proxy::onGlyphMaskNeedsCurrentColor() const {
    return fRealTypeface->glyphMaskNeedsCurrentColor();
}

int SkTypeface_proxy::onGetVariationDesignPosition(
                       SkSpan<SkFontArguments::VariationPosition::Coordinate> coordinates) const {
    return fRealTypeface->onGetVariationDesignPosition(coordinates);
}

int SkTypeface_proxy::onGetVariationDesignParameters(
                                     SkSpan<SkFontParameters::Variation::Axis> parameters) const {
    return fRealTypeface->onGetVariationDesignParameters(parameters);
}

SkFontStyle SkTypeface_proxy::onGetFontStyle() const {
    return fRealTypeface->onGetFontStyle();
}

bool SkTypeface_proxy::onGetFixedPitch() const {
    return fRealTypeface->onGetFixedPitch();
}

void SkTypeface_proxy::onGetFamilyName(SkString* familyName) const {
    fRealTypeface->onGetFamilyName(familyName);
}

bool SkTypeface_proxy::onGetPostScriptName(SkString* postScriptName) const {
    return fRealTypeface->getPostScriptName(postScriptName);
}

int SkTypeface_proxy::onGetResourceName(SkString* resourceName) const {
    return fRealTypeface->getResourceName(resourceName);
}

SkTypeface::LocalizedStrings* SkTypeface_proxy::onCreateFamilyNameIterator() const {
    return fRealTypeface->createFamilyNameIterator();
}

int SkTypeface_proxy::onGetTableTags(SkSpan<SkFontTableTag> tags) const {
    return fRealTypeface->readTableTags(tags);
}

size_t SkTypeface_proxy::onGetTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const {
    return fRealTypeface->getTableData(tag, offset, length, data);
}

std::unique_ptr<SkScalerContext> SkTypeface_proxy::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    return std::make_unique<SkScalerContext_proxy>(
            fRealTypeface->onCreateScalerContextAsProxyTypeface(effects, desc, const_cast<SkTypeface_proxy*>(this)),
            *const_cast<SkTypeface_proxy*>(this),
            effects,
            desc);
}

void SkTypeface_proxy::onFilterRec(SkScalerContextRec* rec) const {
    fRealTypeface->onFilterRec(rec);
}

void SkTypeface_proxy::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    fRealTypeface->onGetFontDescriptor(desc, serialize);
}

void SkTypeface_proxy::getGlyphToUnicodeMap(SkSpan<SkUnichar> glyphToUnicode) const {
    fRealTypeface->getGlyphToUnicodeMap(glyphToUnicode);
}

void SkTypeface_proxy::getPostScriptGlyphNames(SkString* names) const {
    return fRealTypeface->getPostScriptGlyphNames(names);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_proxy::onGetAdvancedMetrics() const {
    return fRealTypeface->onGetAdvancedMetrics();
}

void SkTypeface_proxy::onCharsToGlyphs(SkSpan<const SkUnichar> chars,
                                       SkSpan<SkGlyphID> glyphs) const {
    fRealTypeface->unicharsToGlyphs(chars, glyphs);
}

int SkTypeface_proxy::onCountGlyphs() const { return fRealTypeface->countGlyphs(); }

void* SkTypeface_proxy::onGetCTFontRef() const { return fRealTypeface->onGetCTFontRef(); }

bool SkTypeface_proxy::onGetKerningPairAdjustments(SkSpan<const SkGlyphID> glyphs,
                                                   SkSpan<int32_t> adjustments) const  {
    return fRealTypeface->onGetKerningPairAdjustments(glyphs, adjustments);
}

SkScalerContext_proxy::SkScalerContext_proxy(std::unique_ptr<SkScalerContext> realScalerContext,
                                             SkTypeface_proxy& proxyTypeface,
                                             const SkScalerContextEffects& effects,
                                             const SkDescriptor* desc)
        : SkScalerContext(proxyTypeface, effects, desc)
        , fRealScalerContext(std::move(realScalerContext))
{ }

SkScalerContext::GlyphMetrics SkScalerContext_proxy::generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) {
    return fRealScalerContext->generateMetrics(glyph, alloc);
}

void SkScalerContext_proxy::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    fRealScalerContext->generateImage(glyph, imageBuffer);
}

std::optional<SkScalerContext::GeneratedPath>
SkScalerContext_proxy::generatePath(const SkGlyph& glyph) {
    return fRealScalerContext->generatePath(glyph);
}

sk_sp<SkDrawable> SkScalerContext_proxy::generateDrawable(const SkGlyph& glyph) {
    return fRealScalerContext->generateDrawable(glyph);
}

void SkScalerContext_proxy::generateFontMetrics(SkFontMetrics* metrics) {
    fRealScalerContext->generateFontMetrics(metrics);
}
