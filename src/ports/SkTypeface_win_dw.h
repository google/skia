/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_win_dw_DEFINED
#define SkTypeface_win_dw_DEFINED

#include "include/core/SkTypeface.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkLeanWindows.h"
#include "src/core/SkTypefaceCache.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>
#include <dwrite_1.h>
#include <dwrite_2.h>
#include <dwrite_3.h>

class SkFontDescriptor;
struct SkScalerContextRec;

static SkFontStyle get_style(IDWriteFont* font) {
    int weight = font->GetWeight();
    int width = font->GetStretch();
    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    switch (font->GetStyle()) {
        case DWRITE_FONT_STYLE_NORMAL: slant = SkFontStyle::kUpright_Slant; break;
        case DWRITE_FONT_STYLE_OBLIQUE: slant = SkFontStyle::kOblique_Slant; break;
        case DWRITE_FONT_STYLE_ITALIC: slant = SkFontStyle::kItalic_Slant; break;
        default: SkASSERT(false); break;
    }
    return SkFontStyle(weight, width, slant);
}

class DWriteFontTypeface : public SkTypeface {
private:
    DWriteFontTypeface(const SkFontStyle& style,
                       IDWriteFactory* factory,
                       IDWriteFontFace* fontFace,
                       IDWriteFont* font,
                       IDWriteFontFamily* fontFamily,
                       IDWriteFontFileLoader* fontFileLoader = nullptr,
                       IDWriteFontCollectionLoader* fontCollectionLoader = nullptr)
        : SkTypeface(style, false)
        , fFactory(SkRefComPtr(factory))
        , fDWriteFontCollectionLoader(SkSafeRefComPtr(fontCollectionLoader))
        , fDWriteFontFileLoader(SkSafeRefComPtr(fontFileLoader))
        , fDWriteFontFamily(SkRefComPtr(fontFamily))
        , fDWriteFont(SkRefComPtr(font))
        , fDWriteFontFace(SkRefComPtr(fontFace))
    {
        if (!SUCCEEDED(fDWriteFontFace->QueryInterface(&fDWriteFontFace1))) {
            // IUnknown::QueryInterface states that if it fails, punk will be set to nullptr.
            // http://blogs.msdn.com/b/oldnewthing/archive/2004/03/26/96777.aspx
            SkASSERT_RELEASE(nullptr == fDWriteFontFace1.get());
        }
        if (!SUCCEEDED(fDWriteFontFace->QueryInterface(&fDWriteFontFace2))) {
            SkASSERT_RELEASE(nullptr == fDWriteFontFace2.get());
        }
        if (!SUCCEEDED(fDWriteFontFace->QueryInterface(&fDWriteFontFace4))) {
            SkASSERT_RELEASE(nullptr == fDWriteFontFace4.get());
        }
        if (!SUCCEEDED(fFactory->QueryInterface(&fFactory2))) {
            SkASSERT_RELEASE(nullptr == fFactory2.get());
        }

        if (fDWriteFontFace1 && fDWriteFontFace1->IsMonospacedFont()) {
            this->setIsFixedPitch(true);
        }
    }

public:
    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFactory2> fFactory2;
    SkTScopedComPtr<IDWriteFontCollectionLoader> fDWriteFontCollectionLoader;
    SkTScopedComPtr<IDWriteFontFileLoader> fDWriteFontFileLoader;
    SkTScopedComPtr<IDWriteFontFamily> fDWriteFontFamily;
    SkTScopedComPtr<IDWriteFont> fDWriteFont;
    SkTScopedComPtr<IDWriteFontFace> fDWriteFontFace;
    SkTScopedComPtr<IDWriteFontFace1> fDWriteFontFace1;
    SkTScopedComPtr<IDWriteFontFace2> fDWriteFontFace2;
    SkTScopedComPtr<IDWriteFontFace4> fDWriteFontFace4;

    static sk_sp<DWriteFontTypeface> Make(
        IDWriteFactory* factory,
        IDWriteFontFace* fontFace,
        IDWriteFont* font,
        IDWriteFontFamily* fontFamily,
        IDWriteFontFileLoader* fontFileLoader = nullptr,
        IDWriteFontCollectionLoader* fontCollectionLoader = nullptr)
    {
        return sk_sp<DWriteFontTypeface>(
            new DWriteFontTypeface(get_style(font), factory, fontFace, font, fontFamily,
                                   fontFileLoader, fontCollectionLoader));
    }

protected:
    void weak_dispose() const override {
        if (fDWriteFontCollectionLoader.get()) {
            HRV(fFactory->UnregisterFontCollectionLoader(fDWriteFontCollectionLoader.get()));
        }
        if (fDWriteFontFileLoader.get()) {
            HRV(fFactory->UnregisterFontFileLoader(fDWriteFontFileLoader.get()));
        }

        //SkTypefaceCache::Remove(this);
        INHERITED::weak_dispose();
    }

    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor*) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    void getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    void getPostScriptGlyphNames(SkString*) const override;
    int onGetUPEM() const override;
    void onGetFamilyName(SkString* familyName) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t offset, size_t length, void* data) const override;
    sk_sp<SkData> onCopyTableData(SkFontTableTag) const override;

private:
    typedef SkTypeface INHERITED;
};

#endif
