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
    sk_sp<SkTypeface>              onMakeClone(const SkFontArguments& args) const override {
        return sk_ref_sp(this);
    }
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc) const override {
        return SkScalerContext::MakeEmptyContext(
                sk_ref_sp(const_cast<TestEmptyTypeface*>(this)), effects, desc);

    }
    void                                       onFilterRec(SkScalerContextRec*) const override {}
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }
    void        onGetFontDescriptor(SkFontDescriptor*, bool*) const override {}
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override {
        sk_bzero(glyphs, count * sizeof(glyphs[0]));
    }
    int onCountGlyphs() const override { return 0; }
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkUnichar*) const override {}
    int onGetUPEM() const override { return 0; }
    class EmptyLocalizedStrings : public SkTypeface::LocalizedStrings {
    public:
        bool next(SkTypeface::LocalizedString*) override { return false; }
    };
    void onGetFamilyName(SkString* familyName) const override { familyName->reset(); }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        return new EmptyLocalizedStrings;
    }
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override {
        return 0;
    }
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override {
        return 0;
    }
    int    onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }
};

#endif  // TestEmptyTypeface_DEFINED
