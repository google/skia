/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/win/SkDWriteNTDDI_VERSION.h"

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkEndian.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkTypeface_win_dw.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkDWriteFontFileStream.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkObjBase.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>
#include <dwrite_2.h>
#include <dwrite_3.h>

using namespace skia_private;

namespace {

// Korean fonts Gulim, Dotum, Batang, Gungsuh have bitmap strikes that get
// artifically emboldened by Windows without antialiasing. Korean users prefer
// these over the synthetic boldening performed by Skia. So let's make an
// exception for fonts with bitmap strikes and allow passing through Windows
// simulations for those, until Skia provides more control over simulations in
// font matching, see https://crbug.com/1258378
bool HasBitmapStrikes(const SkTScopedComPtr<IDWriteFont>& font) {
  SkTScopedComPtr<IDWriteFontFace> fontFace;
  HRB(font->CreateFontFace(&fontFace));

  AutoDWriteTable ebdtTable(fontFace.get(),
                            SkEndian_SwapBE32(SkSetFourByteTag('E', 'B', 'D', 'T')));
  return ebdtTable.fExists;
}

// Iterate calls to GetFirstMatchingFont incrementally removing bold or italic
// styling that can trigger the simulations. Implementing it this way gets us a
// IDWriteFont that can be used as before and has the correct information on its
// own style. Stripping simulations from IDWriteFontFace is possible via
// IDWriteFontList1, IDWriteFontFaceReference and CreateFontFace, but this way
// we won't have a matching IDWriteFont which is still used in get_style().
HRESULT FirstMatchingFontWithoutSimulations(const SkTScopedComPtr<IDWriteFontFamily>& family,
                                            DWriteStyle dwStyle,
                                            SkTScopedComPtr<IDWriteFont>& font) {
    bool noSimulations = false;
    while (!noSimulations) {
        SkTScopedComPtr<IDWriteFont> searchFont;
        HR(family->GetFirstMatchingFont(
                dwStyle.fWeight, dwStyle.fWidth, dwStyle.fSlant, &searchFont));
        DWRITE_FONT_SIMULATIONS simulations = searchFont->GetSimulations();
        // If we still get simulations even though we're not asking for bold or
        // italic, we can't help it and exit the loop.

#ifdef SK_WIN_FONTMGR_NO_SIMULATIONS
        noSimulations = simulations == DWRITE_FONT_SIMULATIONS_NONE ||
                        (dwStyle.fWeight == DWRITE_FONT_WEIGHT_REGULAR &&
                         dwStyle.fSlant == DWRITE_FONT_STYLE_NORMAL) ||
                        HasBitmapStrikes(searchFont);
#else
        noSimulations = true;
#endif
        if (noSimulations) {
            font = std::move(searchFont);
            break;
        }
        if (simulations & DWRITE_FONT_SIMULATIONS_BOLD) {
            dwStyle.fWeight = DWRITE_FONT_WEIGHT_REGULAR;
            continue;
        }
        if (simulations & DWRITE_FONT_SIMULATIONS_OBLIQUE) {
            dwStyle.fSlant = DWRITE_FONT_STYLE_NORMAL;
            continue;
        }
    }
    return S_OK;
}
}

////////////////////////////////////////////////////////////////////////////////

class SkFontMgr_DirectWrite : public SkFontMgr {
public:
    /** localeNameLength and defaultFamilyNameLength must include the null terminator. */
    SkFontMgr_DirectWrite(IDWriteFactory* factory, IDWriteFontCollection* fontCollection,
                          IDWriteFontFallback* fallback,
                          const WCHAR* localeName, int localeNameLength,
                          const WCHAR* defaultFamilyName, int defaultFamilyNameLength)
        : fFactory(SkRefComPtr(factory))
        , fFontFallback(SkSafeRefComPtr(fallback))
        , fFontCollection(SkRefComPtr(fontCollection))
        , fLocaleName(localeNameLength)
        , fDefaultFamilyName(defaultFamilyNameLength)
    {
        memcpy(fLocaleName.get(), localeName, localeNameLength * sizeof(WCHAR));
        memcpy(fDefaultFamilyName.get(), defaultFamilyName, defaultFamilyNameLength*sizeof(WCHAR));
    }

protected:
    int onCountFamilies() const override;
    void onGetFamilyName(int index, SkString* familyName) const override;
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override;
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override;
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& fontstyle) const override;
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                                  const char* bcp47[], int bcp47Count,
                                                  SkUnichar character) const override;
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>, const SkFontArguments&) const override;
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override;

private:
    HRESULT getByFamilyName(const WCHAR familyName[], IDWriteFontFamily** fontFamily) const;
    sk_sp<SkTypeface> fallback(const WCHAR* dwFamilyName, DWriteStyle,
                               const WCHAR* dwBcp47, UINT32 character) const;
    sk_sp<SkTypeface> layoutFallback(const WCHAR* dwFamilyName, DWriteStyle,
                                     const WCHAR* dwBcp47, UINT32 character) const;

    /** Creates a typeface using a typeface cache. */
    sk_sp<SkTypeface> makeTypefaceFromDWriteFont(IDWriteFontFace* fontFace,
                                                 IDWriteFont* font,
                                                 IDWriteFontFamily* fontFamily) const;

    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFontFallback> fFontFallback;
    SkTScopedComPtr<IDWriteFontCollection> fFontCollection;
    SkSMallocWCHAR fLocaleName;
    SkSMallocWCHAR fDefaultFamilyName;
    mutable SkMutex fTFCacheMutex;
    mutable SkTypefaceCache fTFCache;

    friend class SkFontStyleSet_DirectWrite;
    friend class FontFallbackRenderer;
};

class SkFontStyleSet_DirectWrite : public SkFontStyleSet {
public:
    SkFontStyleSet_DirectWrite(const SkFontMgr_DirectWrite* fontMgr,
                               IDWriteFontFamily* fontFamily)
        : fFontMgr(SkRef(fontMgr))
        , fFontFamily(SkRefComPtr(fontFamily))
    { }

    int count() override;
    void getStyle(int index, SkFontStyle* fs, SkString* styleName) override;
    sk_sp<SkTypeface> createTypeface(int index) override;
    sk_sp<SkTypeface> matchStyle(const SkFontStyle& pattern) override;

private:
    sk_sp<const SkFontMgr_DirectWrite> fFontMgr;
    SkTScopedComPtr<IDWriteFontFamily> fFontFamily;
};

static HRESULT are_same(IUnknown* a, IUnknown* b, bool& same) {
    SkTScopedComPtr<IUnknown> iunkA;
    HRM(a->QueryInterface(&iunkA), "Failed to QI<IUnknown> for a.");

    SkTScopedComPtr<IUnknown> iunkB;
    HRM(b->QueryInterface(&iunkB), "Failed to QI<IUnknown> for b.");

    same = (iunkA.get() == iunkB.get());
    return S_OK;
}

struct ProtoDWriteTypeface {
    IDWriteFontFace* fDWriteFontFace;
    IDWriteFont* fDWriteFont;
    IDWriteFontFamily* fDWriteFontFamily;
};

static bool FindByDWriteFont(SkTypeface* cached, void* ctx) {
    DWriteFontTypeface* cshFace = reinterpret_cast<DWriteFontTypeface*>(cached);
    ProtoDWriteTypeface* ctxFace = reinterpret_cast<ProtoDWriteTypeface*>(ctx);

    // IDWriteFontFace5 introduced both Equals and HasVariations
    SkTScopedComPtr<IDWriteFontFace5> cshFontFace5;
    SkTScopedComPtr<IDWriteFontFace5> ctxFontFace5;
    cshFace->fDWriteFontFace->QueryInterface(&cshFontFace5);
    ctxFace->fDWriteFontFace->QueryInterface(&ctxFontFace5);
    if (cshFontFace5 && ctxFontFace5) {
        return cshFontFace5->Equals(ctxFontFace5.get());
    }

    bool same;

    //Check to see if the two fonts are identical.
    HRB(are_same(cshFace->fDWriteFont.get(), ctxFace->fDWriteFont, same));
    if (same) {
        return true;
    }

    HRB(are_same(cshFace->fDWriteFontFace.get(), ctxFace->fDWriteFontFace, same));
    if (same) {
        return true;
    }

    //Check if the two fonts share the same loader and have the same key.
    UINT32 cshNumFiles;
    UINT32 ctxNumFiles;
    HRB(cshFace->fDWriteFontFace->GetFiles(&cshNumFiles, nullptr));
    HRB(ctxFace->fDWriteFontFace->GetFiles(&ctxNumFiles, nullptr));
    if (cshNumFiles != ctxNumFiles) {
        return false;
    }

    SkTScopedComPtr<IDWriteFontFile> cshFontFile;
    SkTScopedComPtr<IDWriteFontFile> ctxFontFile;
    HRB(cshFace->fDWriteFontFace->GetFiles(&cshNumFiles, &cshFontFile));
    HRB(ctxFace->fDWriteFontFace->GetFiles(&ctxNumFiles, &ctxFontFile));

    //for (each file) { //we currently only admit fonts from one file.
    SkTScopedComPtr<IDWriteFontFileLoader> cshFontFileLoader;
    SkTScopedComPtr<IDWriteFontFileLoader> ctxFontFileLoader;
    HRB(cshFontFile->GetLoader(&cshFontFileLoader));
    HRB(ctxFontFile->GetLoader(&ctxFontFileLoader));
    HRB(are_same(cshFontFileLoader.get(), ctxFontFileLoader.get(), same));
    if (!same) {
        return false;
    }
    //}

    const void* cshRefKey;
    UINT32 cshRefKeySize;
    const void* ctxRefKey;
    UINT32 ctxRefKeySize;
    HRB(cshFontFile->GetReferenceKey(&cshRefKey, &cshRefKeySize));
    HRB(ctxFontFile->GetReferenceKey(&ctxRefKey, &ctxRefKeySize));
    if (cshRefKeySize != ctxRefKeySize) {
        return false;
    }
    if (0 != memcmp(cshRefKey, ctxRefKey, ctxRefKeySize)) {
        return false;
    }

    //TODO: better means than comparing name strings?
    //NOTE: .ttc and fake bold/italic will end up here.
    SkTScopedComPtr<IDWriteLocalizedStrings> cshFamilyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> cshFaceNames;
    HRB(cshFace->fDWriteFontFamily->GetFamilyNames(&cshFamilyNames));
    HRB(cshFace->fDWriteFont->GetFaceNames(&cshFaceNames));
    UINT32 cshFamilyNameLength;
    UINT32 cshFaceNameLength;
    HRB(cshFamilyNames->GetStringLength(0, &cshFamilyNameLength));
    HRB(cshFaceNames->GetStringLength(0, &cshFaceNameLength));

    SkTScopedComPtr<IDWriteLocalizedStrings> ctxFamilyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> ctxFaceNames;
    HRB(ctxFace->fDWriteFontFamily->GetFamilyNames(&ctxFamilyNames));
    HRB(ctxFace->fDWriteFont->GetFaceNames(&ctxFaceNames));
    UINT32 ctxFamilyNameLength;
    UINT32 ctxFaceNameLength;
    HRB(ctxFamilyNames->GetStringLength(0, &ctxFamilyNameLength));
    HRB(ctxFaceNames->GetStringLength(0, &ctxFaceNameLength));

    if (cshFamilyNameLength != ctxFamilyNameLength ||
        cshFaceNameLength != ctxFaceNameLength)
    {
        return false;
    }

    SkSMallocWCHAR cshFamilyName(cshFamilyNameLength+1);
    SkSMallocWCHAR cshFaceName(cshFaceNameLength+1);
    HRB(cshFamilyNames->GetString(0, cshFamilyName.get(), cshFamilyNameLength+1));
    HRB(cshFaceNames->GetString(0, cshFaceName.get(), cshFaceNameLength+1));

    SkSMallocWCHAR ctxFamilyName(ctxFamilyNameLength+1);
    SkSMallocWCHAR ctxFaceName(ctxFaceNameLength+1);
    HRB(ctxFamilyNames->GetString(0, ctxFamilyName.get(), ctxFamilyNameLength+1));
    HRB(ctxFaceNames->GetString(0, ctxFaceName.get(), ctxFaceNameLength+1));

    return wcscmp(cshFamilyName.get(), ctxFamilyName.get()) == 0 &&
           wcscmp(cshFaceName.get(), ctxFaceName.get()) == 0;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::makeTypefaceFromDWriteFont(
        IDWriteFontFace* fontFace,
        IDWriteFont* font,
        IDWriteFontFamily* fontFamily) const {
    SkAutoMutexExclusive ama(fTFCacheMutex);
    ProtoDWriteTypeface spec = { fontFace, font, fontFamily };
    sk_sp<SkTypeface> face = fTFCache.findByProcAndRef(FindByDWriteFont, &spec);
    if (nullptr == face) {
        face = DWriteFontTypeface::Make(fFactory.get(), fontFace, font, fontFamily, nullptr,
                                        SkFontArguments::Palette{0, nullptr, 0});
        if (face) {
            fTFCache.add(face);
        }
    }
    return face;
}

int SkFontMgr_DirectWrite::onCountFamilies() const {
    return fFontCollection->GetFontFamilyCount();
}

void SkFontMgr_DirectWrite::onGetFamilyName(int index, SkString* familyName) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRVM(fFontCollection->GetFontFamily(index, &fontFamily), "Could not get requested family.");

    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRVM(fontFamily->GetFamilyNames(&familyNames), "Could not get family names.");

    sk_get_locale_string(familyNames.get(), fLocaleName.get(), familyName);
}

sk_sp<SkFontStyleSet> SkFontMgr_DirectWrite::onCreateStyleSet(int index) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRNM(fFontCollection->GetFontFamily(index, &fontFamily), "Could not get requested family.");

    return sk_sp<SkFontStyleSet>(new SkFontStyleSet_DirectWrite(this, fontFamily.get()));
}

sk_sp<SkFontStyleSet> SkFontMgr_DirectWrite::onMatchFamily(const char familyName[]) const {
    if (!familyName) {
        return nullptr;
    }

    SkSMallocWCHAR dwFamilyName;
    HRN(sk_cstring_to_wchar(familyName, &dwFamilyName));

    UINT32 index;
    BOOL exists;
    HRNM(fFontCollection->FindFamilyName(dwFamilyName.get(), &index, &exists),
            "Failed while finding family by name.");
    if (!exists) {
        return nullptr;
    }

    return this->onCreateStyleSet(index);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMatchFamilyStyle(const char familyName[],
                                                            const SkFontStyle& fontstyle) const {
    sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
    return sset->matchStyle(fontstyle);
}

class FontFallbackRenderer : public IDWriteTextRenderer {
public:
    FontFallbackRenderer(const SkFontMgr_DirectWrite* outer, UINT32 character)
        : fRefCount(1), fOuter(SkSafeRef(outer)), fCharacter(character), fResolvedTypeface(nullptr) {
    }

    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(IID const& riid, void** ppvObject) override {
        if (__uuidof(IUnknown) == riid ||
            __uuidof(IDWritePixelSnapping) == riid ||
            __uuidof(IDWriteTextRenderer) == riid)
        {
            *ppvObject = this;
            this->AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_FAIL;
    }

    SK_STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&fRefCount);
    }

    SK_STDMETHODIMP_(ULONG) Release() override {
        ULONG newCount = InterlockedDecrement(&fRefCount);
        if (0 == newCount) {
            delete this;
        }
        return newCount;
    }

    // IDWriteTextRenderer methods
    SK_STDMETHODIMP DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect) override
    {
        if (!glyphRun->fontFace) {
            HRM(E_INVALIDARG, "Glyph run without font face.");
        }

        SkTScopedComPtr<IDWriteFont> font;
        HRM(fOuter->fFontCollection->GetFontFromFontFace(glyphRun->fontFace, &font),
            "Could not get font from font face.");

        // It is possible that the font passed does not actually have the requested character,
        // due to no font being found and getting the fallback font.
        // Check that the font actually contains the requested character.
        BOOL exists;
        HRM(font->HasCharacter(fCharacter, &exists), "Could not find character.");

        if (exists) {
            SkTScopedComPtr<IDWriteFontFamily> fontFamily;
            HRM(font->GetFontFamily(&fontFamily), "Could not get family.");
            fResolvedTypeface = fOuter->makeTypefaceFromDWriteFont(glyphRun->fontFace,
                                                                   font.get(),
                                                                   fontFamily.get());
            fHasSimulations = (font->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE) &&
                              !HasBitmapStrikes(font);
        }

        return S_OK;
    }

    SK_STDMETHODIMP DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect) override
    { return E_NOTIMPL; }

    SK_STDMETHODIMP DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect) override
    { return E_NOTIMPL; }

    SK_STDMETHODIMP DrawInlineObject(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect) override
    { return E_NOTIMPL; }

    // IDWritePixelSnapping methods
    SK_STDMETHODIMP IsPixelSnappingDisabled(
        void* clientDrawingContext,
        BOOL* isDisabled) override
    {
        *isDisabled = FALSE;
        return S_OK;
    }

    SK_STDMETHODIMP GetCurrentTransform(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform) override
    {
        const DWRITE_MATRIX ident = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
        *transform = ident;
        return S_OK;
    }

    SK_STDMETHODIMP GetPixelsPerDip(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip) override
    {
        *pixelsPerDip = 1.0f;
        return S_OK;
    }

    sk_sp<SkTypeface> ConsumeFallbackTypeface() { return std::move(fResolvedTypeface); }

    bool FallbackTypefaceHasSimulations() { return fHasSimulations; }

private:
    virtual ~FontFallbackRenderer() { }

    ULONG fRefCount;
    sk_sp<const SkFontMgr_DirectWrite> fOuter;
    UINT32 fCharacter;
    sk_sp<SkTypeface> fResolvedTypeface;
    bool fHasSimulations{false};
};

class FontFallbackSource : public IDWriteTextAnalysisSource {
public:
    FontFallbackSource(const WCHAR* string, UINT32 length, const WCHAR* locale,
                       IDWriteNumberSubstitution* numberSubstitution)
        : fRefCount(1)
        , fString(string)
        , fLength(length)
        , fLocale(locale)
        , fNumberSubstitution(numberSubstitution)
    { }

    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(IID const& riid, void** ppvObject) override {
        if (__uuidof(IUnknown) == riid ||
            __uuidof(IDWriteTextAnalysisSource) == riid)
        {
            *ppvObject = this;
            this->AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_FAIL;
    }

    SK_STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&fRefCount);
    }

    SK_STDMETHODIMP_(ULONG) Release() override {
        ULONG newCount = InterlockedDecrement(&fRefCount);
        if (0 == newCount) {
            delete this;
        }
        return newCount;
    }

    // IDWriteTextAnalysisSource methods
    SK_STDMETHODIMP GetTextAtPosition(
        UINT32 textPosition,
        WCHAR const** textString,
        UINT32* textLength) override
    {
        if (fLength <= textPosition) {
            *textString = nullptr;
            *textLength = 0;
            return S_OK;
        }
        *textString = fString + textPosition;
        *textLength = fLength - textPosition;
        return S_OK;
    }

    SK_STDMETHODIMP GetTextBeforePosition(
        UINT32 textPosition,
        WCHAR const** textString,
        UINT32* textLength) override
    {
        if (textPosition < 1 || fLength <= textPosition) {
            *textString = nullptr;
            *textLength = 0;
            return S_OK;
        }
        *textString = fString;
        *textLength = textPosition;
        return S_OK;
    }

    SK_STDMETHODIMP_(DWRITE_READING_DIRECTION) GetParagraphReadingDirection() override {
        // TODO: this is also interesting.
        return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
    }

    SK_STDMETHODIMP GetLocaleName(
        UINT32 textPosition,
        UINT32* textLength,
        WCHAR const** localeName) override
    {
        *localeName = fLocale;
        return S_OK;
    }

    SK_STDMETHODIMP GetNumberSubstitution(
        UINT32 textPosition,
        UINT32* textLength,
        IDWriteNumberSubstitution** numberSubstitution) override
    {
        *numberSubstitution = fNumberSubstitution;
        return S_OK;
    }

private:
    virtual ~FontFallbackSource() { }

    ULONG fRefCount;
    const WCHAR* fString;
    UINT32 fLength;
    const WCHAR* fLocale;
    IDWriteNumberSubstitution* fNumberSubstitution;
};

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMatchFamilyStyleCharacter(
    const char familyName[], const SkFontStyle& style,
    const char* bcp47[], int bcp47Count,
    SkUnichar character) const
{
    DWriteStyle dwStyle(style);

    const WCHAR* dwFamilyName = nullptr;
    SkSMallocWCHAR dwFamilyNameLocal;
    if (familyName) {
        HRN(sk_cstring_to_wchar(familyName, &dwFamilyNameLocal));
        dwFamilyName = dwFamilyNameLocal;
    }

    const SkSMallocWCHAR* dwBcp47;
    SkSMallocWCHAR dwBcp47Local;
    if (bcp47Count < 1) {
        dwBcp47 = &fLocaleName;
    } else {
        // TODO: support fallback stack.
        // TODO: DirectWrite supports 'zh-CN' or 'zh-Hans', but 'zh' misses completely
        // and may produce a Japanese font.
        HRN(sk_cstring_to_wchar(bcp47[bcp47Count - 1], &dwBcp47Local));
        dwBcp47 = &dwBcp47Local;
    }

    if (fFontFallback) {
        return this->fallback(dwFamilyName, dwStyle, dwBcp47->get(), character);
    }

    // LayoutFallback may use the system font collection for fallback.
    return this->layoutFallback(dwFamilyName, dwStyle, dwBcp47->get(), character);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::fallback(const WCHAR* dwFamilyName,
                                                  DWriteStyle dwStyle,
                                                  const WCHAR* dwBcp47,
                                                  UINT32 character) const {
    WCHAR str[16];
    UINT32 strLen = SkTo<UINT32>(SkUTF::ToUTF16(character, reinterpret_cast<uint16_t*>(str)));

    if (!fFontFallback) {
        return nullptr;
    }

    SkTScopedComPtr<IDWriteNumberSubstitution> numberSubstitution;
    HRNM(fFactory->CreateNumberSubstitution(DWRITE_NUMBER_SUBSTITUTION_METHOD_NONE, dwBcp47,
                                            TRUE, &numberSubstitution),
         "Could not create number substitution.");
    SkTScopedComPtr<FontFallbackSource> fontFallbackSource(
        new FontFallbackSource(str, strLen, dwBcp47, numberSubstitution.get()));

    UINT32 mappedLength;
    SkTScopedComPtr<IDWriteFont> font;
    FLOAT scale;

    bool noSimulations = false;
    while (!noSimulations) {
        font.reset();
        HRNM(fFontFallback->MapCharacters(fontFallbackSource.get(),
                                          0,  // textPosition,
                                          strLen,
                                          fFontCollection.get(),
                                          dwFamilyName,
                                          dwStyle.fWeight,
                                          dwStyle.fSlant,
                                          dwStyle.fWidth,
                                          &mappedLength,
                                          &font,
                                          &scale),
             "Could not map characters");
        if (!font.get()) {
            return nullptr;
        }

        DWRITE_FONT_SIMULATIONS simulations = font->GetSimulations();

#ifdef SK_WIN_FONTMGR_NO_SIMULATIONS
        noSimulations = simulations == DWRITE_FONT_SIMULATIONS_NONE || HasBitmapStrikes(font);
#else
        noSimulations = true;
#endif

        if (simulations & DWRITE_FONT_SIMULATIONS_BOLD) {
            dwStyle.fWeight = DWRITE_FONT_WEIGHT_REGULAR;
            continue;
        }

        if (simulations & DWRITE_FONT_SIMULATIONS_OBLIQUE) {
            dwStyle.fSlant = DWRITE_FONT_STYLE_NORMAL;
            continue;
        }
    }

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not get font face from font.");

    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRNM(font->GetFontFamily(&fontFamily), "Could not get family from font.");
    return this->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get());
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::layoutFallback(const WCHAR* dwFamilyName,
                                                        DWriteStyle dwStyle,
                                                        const WCHAR* dwBcp47,
                                                        UINT32 character) const
{
    WCHAR str[16];
    UINT32 strLen = SkTo<UINT32>(SkUTF::ToUTF16(character, reinterpret_cast<uint16_t*>(str)));

    bool noSimulations = false;
    sk_sp<SkTypeface> returnTypeface(nullptr);
    while (!noSimulations) {
        SkTScopedComPtr<IDWriteTextFormat> fallbackFormat;
        HRNM(fFactory->CreateTextFormat(dwFamilyName ? dwFamilyName : L"",
                                        fFontCollection.get(),
                                        dwStyle.fWeight,
                                        dwStyle.fSlant,
                                        dwStyle.fWidth,
                                        72.0f,
                                        dwBcp47,
                                        &fallbackFormat),
             "Could not create text format.");

        // No matter how the font collection is set on this IDWriteTextLayout, it is not possible to
        // disable use of the system font collection in fallback.
        SkTScopedComPtr<IDWriteTextLayout> fallbackLayout;
        HRNM(fFactory->CreateTextLayout(
                     str, strLen, fallbackFormat.get(), 200.0f, 200.0f, &fallbackLayout),
             "Could not create text layout.");

        SkTScopedComPtr<FontFallbackRenderer> fontFallbackRenderer(
                new FontFallbackRenderer(this, character));

        HRNM(fallbackLayout->SetFontCollection(fFontCollection.get(), {0, strLen}),
             "Could not set layout font collection.");
        HRNM(fallbackLayout->Draw(nullptr, fontFallbackRenderer.get(), 50.0f, 50.0f),
             "Could not draw layout with renderer.");

#ifdef SK_WIN_FONTMGR_NO_SIMULATIONS
        noSimulations = !fontFallbackRenderer->FallbackTypefaceHasSimulations();
#else
        noSimulations = true;
#endif

        if (noSimulations) {
            returnTypeface = fontFallbackRenderer->ConsumeFallbackTypeface();
        }

        if (dwStyle.fWeight != DWRITE_FONT_WEIGHT_REGULAR) {
            dwStyle.fWeight = DWRITE_FONT_WEIGHT_REGULAR;
            continue;
        }

        if (dwStyle.fSlant != DWRITE_FONT_STYLE_NORMAL) {
            dwStyle.fSlant = DWRITE_FONT_STYLE_NORMAL;
            continue;
        }
    }

    return returnTypeface;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                                               int ttcIndex) const {
    SkFontArguments args;
    args.setCollectionIndex(ttcIndex);
    return this->onMakeFromStreamArgs(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                              const SkFontArguments& args) const {
    return DWriteFontTypeface::MakeFromStream(std::move(stream), args);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const {
    return this->makeFromStream(std::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromFile(const char path[], int ttcIndex) const {
    return this->makeFromStream(SkStream::MakeFromFile(path), ttcIndex);
}

HRESULT SkFontMgr_DirectWrite::getByFamilyName(const WCHAR wideFamilyName[],
                                               IDWriteFontFamily** fontFamily) const {
    UINT32 index;
    BOOL exists;
    HR(fFontCollection->FindFamilyName(wideFamilyName, &index, &exists));

    if (exists) {
        HR(fFontCollection->GetFontFamily(index, fontFamily));
    }
    return S_OK;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onLegacyMakeTypeface(const char familyName[],
                                                              SkFontStyle style) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    DWriteStyle dwStyle(style);
    if (familyName) {
        SkSMallocWCHAR dwFamilyName;
        if (SUCCEEDED(sk_cstring_to_wchar(familyName, &dwFamilyName))) {
            this->getByFamilyName(dwFamilyName, &fontFamily);
            if (!fontFamily && fFontFallback) {
                return this->fallback(
                        dwFamilyName, dwStyle, fLocaleName.get(), 32);
            }
        }
    }

    if (!fontFamily) {
        if (fFontFallback) {
            return this->fallback(nullptr, dwStyle, fLocaleName.get(), 32);
        }
        // SPI_GETNONCLIENTMETRICS lfMessageFont can fail in Win8. (DisallowWin32kSystemCalls)
        // layoutFallback causes DCHECK in Chromium. (Uses system font collection.)
        HRNM(this->getByFamilyName(fDefaultFamilyName, &fontFamily),
             "Could not create DWrite font family from LOGFONT.");
    }

    if (!fontFamily) {
        // Could not obtain the default font.
        HRNM(fFontCollection->GetFontFamily(0, &fontFamily),
             "Could not get default-default font family.");
    }

    SkTScopedComPtr<IDWriteFont> font;
    HRNM(FirstMatchingFontWithoutSimulations(fontFamily, dwStyle, font),
         "No font found from family.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return this->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get());
}

///////////////////////////////////////////////////////////////////////////////

int SkFontStyleSet_DirectWrite::count() {
    return fFontFamily->GetFontCount();
}

sk_sp<SkTypeface> SkFontStyleSet_DirectWrite::createTypeface(int index) {
    SkTScopedComPtr<IDWriteFont> font;
    HRNM(fFontFamily->GetFont(index, &font), "Could not get font.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return fFontMgr->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fFontFamily.get());
}

void SkFontStyleSet_DirectWrite::getStyle(int index, SkFontStyle* fs, SkString* styleName) {
    SkTScopedComPtr<IDWriteFont> font;
    HRVM(fFontFamily->GetFont(index, &font), "Could not get font.");

    if (fs) {
        *fs = get_style(font.get());
    }

    if (styleName) {
        SkTScopedComPtr<IDWriteLocalizedStrings> faceNames;
        if (SUCCEEDED(font->GetFaceNames(&faceNames))) {
            sk_get_locale_string(faceNames.get(), fFontMgr->fLocaleName.get(), styleName);
        }
    }
}

sk_sp<SkTypeface> SkFontStyleSet_DirectWrite::matchStyle(const SkFontStyle& pattern) {
    SkTScopedComPtr<IDWriteFont> font;
    DWriteStyle dwStyle(pattern);

    HRNM(FirstMatchingFontWithoutSimulations(fFontFamily, dwStyle, font),
         "No font found from family.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return fFontMgr->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fFontFamily.get());
}

////////////////////////////////////////////////////////////////////////////////
#include "include/ports/SkTypeface_win.h"

sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
                                           IDWriteFontCollection* collection) {
    return SkFontMgr_New_DirectWrite(factory, collection, nullptr);
}

sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
                                           IDWriteFontCollection* collection,
                                           IDWriteFontFallback* fallback) {
    if (nullptr == factory) {
        factory = sk_get_dwrite_factory();
        if (nullptr == factory) {
            return nullptr;
        }
    }

    SkTScopedComPtr<IDWriteFontCollection> systemFontCollection;
    if (nullptr == collection) {
        HRNM(factory->GetSystemFontCollection(&systemFontCollection, FALSE),
             "Could not get system font collection.");
        collection = systemFontCollection.get();
    }

    // It is possible to have been provided a font fallback when factory2 is not available.
    SkTScopedComPtr<IDWriteFontFallback> systemFontFallback;
    if (nullptr == fallback) {
        SkTScopedComPtr<IDWriteFactory2> factory2;
        if (!SUCCEEDED(factory->QueryInterface(&factory2))) {
            // IUnknown::QueryInterface states that if it fails, punk will be set to nullptr.
            // http://blogs.msdn.com/b/oldnewthing/archive/2004/03/26/96777.aspx
            SkASSERT_RELEASE(nullptr == factory2.get());
        } else {
            HRNM(factory2->GetSystemFontFallback(&systemFontFallback),
                 "Could not get system fallback.");
            fallback = systemFontFallback.get();
        }
    }

    const WCHAR* defaultFamilyName = L"";
    int defaultFamilyNameLen = 1;
    NONCLIENTMETRICSW metrics;
    metrics.cbSize = sizeof(metrics);

    #ifndef SK_WINUWP
    if (nullptr == fallback) {
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0)) {
            defaultFamilyName = metrics.lfMessageFont.lfFaceName;
            defaultFamilyNameLen = LF_FACESIZE;
        }
    }
    #endif //SK_WINUWP

    WCHAR localeNameStorage[LOCALE_NAME_MAX_LENGTH];
    const WCHAR* localeName = L"";
    int localeNameLen = 1;

    // Dynamically load GetUserDefaultLocaleName function, as it is not available on XP.
    SkGetUserDefaultLocaleNameProc getUserDefaultLocaleNameProc = nullptr;
    HRESULT hr = SkGetGetUserDefaultLocaleNameProc(&getUserDefaultLocaleNameProc);
    if (nullptr == getUserDefaultLocaleNameProc) {
        SK_TRACEHR(hr, "Could not get GetUserDefaultLocaleName.");
    } else {
        int size = getUserDefaultLocaleNameProc(localeNameStorage, LOCALE_NAME_MAX_LENGTH);
        if (size) {
            localeName = localeNameStorage;
            localeNameLen = size;
        }
    }

    return sk_make_sp<SkFontMgr_DirectWrite>(factory, collection, fallback,
                                             localeName, localeNameLen,
                                             defaultFamilyName, defaultFamilyNameLen);
}

#include "include/ports/SkFontMgr_indirect.h"
sk_sp<SkFontMgr> SkFontMgr_New_DirectWriteRenderer(sk_sp<SkRemotableFontMgr> proxy) {
    sk_sp<SkFontMgr> impl(SkFontMgr_New_DirectWrite());
    if (!impl) {
        return nullptr;
    }
    return sk_make_sp<SkFontMgr_Indirect>(std::move(impl), std::move(proxy));
}
#endif//defined(SK_BUILD_FOR_WIN)
