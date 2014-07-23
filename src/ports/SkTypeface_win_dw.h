/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_win_dw_DEFINED
#define SkTypeface_win_dw_DEFINED

#include "SkAdvancedTypefaceMetrics.h"
#include "SkDWrite.h"
#include "SkHRESULT.h"
#include "SkTScopedComPtr.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkTypes.h"

#include <dwrite.h>
#include <dwrite_1.h>

class SkFontDescriptor;
struct SkScalerContextRec;

static SkTypeface::Style get_style(IDWriteFont* font) {
    int style = SkTypeface::kNormal;
    DWRITE_FONT_WEIGHT weight = font->GetWeight();
    if (DWRITE_FONT_WEIGHT_DEMI_BOLD <= weight) {
        style |= SkTypeface::kBold;
    }
    DWRITE_FONT_STYLE angle = font->GetStyle();
    if (DWRITE_FONT_STYLE_OBLIQUE == angle || DWRITE_FONT_STYLE_ITALIC == angle) {
        style |= SkTypeface::kItalic;
    }
    return static_cast<SkTypeface::Style>(style);
}

class DWriteFontTypeface : public SkTypeface {
private:
    DWriteFontTypeface(SkTypeface::Style style, SkFontID fontID,
                       IDWriteFactory* factory,
                       IDWriteFontFace* fontFace,
                       IDWriteFont* font,
                       IDWriteFontFamily* fontFamily,
                       IDWriteFontFileLoader* fontFileLoader = NULL,
                       IDWriteFontCollectionLoader* fontCollectionLoader = NULL)
        : SkTypeface(style, fontID, false)
        , fFactory(SkRefComPtr(factory))
        , fDWriteFontCollectionLoader(SkSafeRefComPtr(fontCollectionLoader))
        , fDWriteFontFileLoader(SkSafeRefComPtr(fontFileLoader))
        , fDWriteFontFamily(SkRefComPtr(fontFamily))
        , fDWriteFont(SkRefComPtr(font))
        , fDWriteFontFace(SkRefComPtr(fontFace))
    {
        if (!SUCCEEDED(fDWriteFontFace->QueryInterface(&fDWriteFontFace1))) {
            // IUnknown::QueryInterface states that if it fails, punk will be set to NULL.
            // http://blogs.msdn.com/b/oldnewthing/archive/2004/03/26/96777.aspx
            SK_ALWAYSBREAK(NULL == fDWriteFontFace1.get());
        }
    }

public:
    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFontCollectionLoader> fDWriteFontCollectionLoader;
    SkTScopedComPtr<IDWriteFontFileLoader> fDWriteFontFileLoader;
    SkTScopedComPtr<IDWriteFontFamily> fDWriteFontFamily;
    SkTScopedComPtr<IDWriteFont> fDWriteFont;
    SkTScopedComPtr<IDWriteFontFace> fDWriteFontFace;
    SkTScopedComPtr<IDWriteFontFace1> fDWriteFontFace1;

    static DWriteFontTypeface* Create(IDWriteFactory* factory,
                                      IDWriteFontFace* fontFace,
                                      IDWriteFont* font,
                                      IDWriteFontFamily* fontFamily,
                                      IDWriteFontFileLoader* fontFileLoader = NULL,
                                      IDWriteFontCollectionLoader* fontCollectionLoader = NULL) {
        SkTypeface::Style style = get_style(font);
        SkFontID fontID = SkTypefaceCache::NewFontID();
        return SkNEW_ARGS(DWriteFontTypeface, (style, fontID,
                                               factory, fontFace, font, fontFamily,
                                               fontFileLoader, fontCollectionLoader));
    }

protected:
    virtual void weak_dispose() const SK_OVERRIDE {
        if (fDWriteFontCollectionLoader.get()) {
            HRV(fFactory->UnregisterFontCollectionLoader(fDWriteFontCollectionLoader.get()));
        }
        if (fDWriteFontFileLoader.get()) {
            HRV(fFactory->UnregisterFontFileLoader(fDWriteFontFileLoader.get()));
        }

        //SkTypefaceCache::Remove(this);
        INHERITED::weak_dispose();
    }

    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE;
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const SK_OVERRIDE;
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                const uint32_t*, uint32_t) const SK_OVERRIDE;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE;
    virtual int onCharsToGlyphs(const void* chars, Encoding encoding,
                                uint16_t glyphs[], int glyphCount) const SK_OVERRIDE;
    virtual int onCountGlyphs() const SK_OVERRIDE;
    virtual int onGetUPEM() const SK_OVERRIDE;
    virtual SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const SK_OVERRIDE;
    virtual int onGetTableTags(SkFontTableTag tags[]) const SK_OVERRIDE;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const SK_OVERRIDE;

private:
    typedef SkTypeface INHERITED;
};

#endif
