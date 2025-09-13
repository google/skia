/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestEmptyTypeface_DEFINED
#define TestEmptyTypeface_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkScalerContext.h"

class TestEmptyTypeface : public SkTypeface {
public:
    static sk_sp<SkTypeface> Make() { return sk_sp<SkTypeface>(new TestEmptyTypeface); }

protected:
    TestEmptyTypeface() : SkTypeface(SkFontStyle(), true) {}

    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override { return nullptr; }
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    std::unique_ptr<SkScalerContext> onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const override
    {
        return SkScalerContext::MakeEmpty(*const_cast<TestEmptyTypeface*>(this), effects, desc);
    }
    void onFilterRec(SkScalerContextRec*) const override {}
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override {}
    void onCharsToGlyphs(SkSpan<const SkUnichar>, SkSpan<SkGlyphID> glyphs) const override {
        sk_bzero(glyphs.data(), glyphs.size_bytes());
    }
    int onCountGlyphs() const override { return 0; }
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkSpan<SkUnichar>) const override {}
    int onGetUPEM() const override { return 0; }
    class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
    public:
        bool next(SkTypeface::LocalizedString*) override { return false; }
    };
    void onGetFamilyName(SkString* familyName) const override { familyName->reset(); }
    bool onGetPostScriptName(SkString*) const override { return false; }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        return new EmptyLocalizedStrings;
    }
    bool onGlyphMaskNeedsCurrentColor() const override { return false; }
    int onGetVariationDesignPosition(
                         SkSpan<SkFontArguments::VariationPosition::Coordinate>) const override {
        return 0;
    }
    int onGetVariationDesignParameters(SkSpan<SkFontParameters::Variation::Axis>) const override {
        return 0;
    }
    int    onGetTableTags(SkSpan<SkFontTableTag>) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }
};

#endif  // TestEmptyTypeface_DEFINED
