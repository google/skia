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

int SkTypeface_proxy::onGetUPEM() const { return fProxy->getUnitsPerEm(); }

std::unique_ptr<SkStreamAsset> SkTypeface_proxy::onOpenStream(int* ttcIndex) const {
    return fProxy->onOpenStream(ttcIndex);
}

sk_sp<SkTypeface> SkTypeface_proxy::onMakeClone(const SkFontArguments& args) const {
    return fProxy->onMakeClone(args);
}

bool SkTypeface_proxy::onGlyphMaskNeedsCurrentColor() const {
    return fProxy->glyphMaskNeedsCurrentColor();
}

int SkTypeface_proxy::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const {
    return fProxy->onGetVariationDesignPosition(coordinates, coordinateCount);
}

int SkTypeface_proxy::onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                                     int parameterCount) const {
    return fProxy->onGetVariationDesignParameters(parameters, parameterCount);
}

SkFontStyle SkTypeface_proxy::onGetFontStyle() const {
    return fProxy->onGetFontStyle();
}

bool SkTypeface_proxy::onGetFixedPitch() const {
    return fProxy->onGetFixedPitch();
}

void SkTypeface_proxy::onGetFamilyName(SkString* familyName) const {
    fProxy->onGetFamilyName(familyName);
}

bool SkTypeface_proxy::onGetPostScriptName(SkString* postScriptName) const {
    return fProxy->getPostScriptName(postScriptName);
}

int SkTypeface_proxy::onGetResourceName(SkString* resourceName) const {
    return fProxy->getResourceName(resourceName);
}

SkTypeface::LocalizedStrings* SkTypeface_proxy::onCreateFamilyNameIterator() const {
    return fProxy->createFamilyNameIterator();
}

int SkTypeface_proxy::onGetTableTags(SkFontTableTag tags[]) const {
    return fProxy->getTableTags(tags);
}

size_t SkTypeface_proxy::onGetTableData(SkFontTableTag tag, size_t offset, size_t length, void* data) const {
    return fProxy->getTableData(tag, offset, length, data);
}

std::unique_ptr<SkScalerContext> SkTypeface_proxy::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    auto proxyScaler = fProxy->onCreateScalerContextAsProxyTypeface(effects, desc, sk_ref_sp(this));
    auto scaler = std::make_unique<SkScalerContext_proxy>(
            std::move(proxyScaler),
            sk_ref_sp(const_cast<SkTypeface_proxy*>(this)),
            effects,
            desc);
    return scaler;
}

void SkTypeface_proxy::onFilterRec(SkScalerContextRec* rec) const {
    fProxy->onFilterRec(rec);
}

void SkTypeface_proxy::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    fProxy->onGetFontDescriptor(desc, serialize);
}

void SkTypeface_proxy::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    fProxy->getGlyphToUnicodeMap(glyphToUnicode);
}

void SkTypeface_proxy::getPostScriptGlyphNames(SkString* names) const {
    return fProxy->getPostScriptGlyphNames(names);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_proxy::onGetAdvancedMetrics() const {
    return fProxy->onGetAdvancedMetrics();
}

void SkTypeface_proxy::onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const {
    fProxy->unicharsToGlyphs(chars, count, glyphs);
}

int SkTypeface_proxy::onCountGlyphs() const { return fProxy->countGlyphs(); }

void* SkTypeface_proxy::onGetCTFontRef() const { return fProxy->onGetCTFontRef(); }

bool SkTypeface_proxy::onGetKerningPairAdjustments(const uint16_t glyphs[], int count,
                                                   int32_t adjustments[]) const  {
    return fProxy->onGetKerningPairAdjustments(glyphs, count, adjustments);
}

SkScalerContext_proxy::SkScalerContext_proxy(std::unique_ptr<SkScalerContext> proxy,
                           sk_sp<SkTypeface_proxy> typeface,
                           const SkScalerContextEffects& effects,
                           const SkDescriptor* desc)
        : SkScalerContext(std::move(typeface), effects, desc)
        , fProxy(std::move(proxy))
{ }

SkScalerContext::GlyphMetrics SkScalerContext_proxy::generateMetrics(const SkGlyph& glyph, SkArenaAlloc* alloc) {
    return fProxy->generateMetrics(glyph, alloc);
}

void SkScalerContext_proxy::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    fProxy->generateImage(glyph, imageBuffer);
}

bool SkScalerContext_proxy::generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) {
    return fProxy->generatePath(glyph, path, modified);
}

sk_sp<SkDrawable> SkScalerContext_proxy::generateDrawable(const SkGlyph& glyph) {
    return fProxy->generateDrawable(glyph);
}

void SkScalerContext_proxy::generateFontMetrics(SkFontMetrics* metrics) {
    fProxy->generateFontMetrics(metrics);
}
