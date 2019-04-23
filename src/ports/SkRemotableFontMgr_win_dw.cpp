/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/win/SkDWriteNTDDI_VERSION.h"

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkRemotableFontMgr.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "src/ports/SkTypeface_win_dw.h"
#include "src/utils/SkUTF.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkDWriteFontFileStream.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>

class SK_API SkRemotableFontMgr_DirectWrite : public SkRemotableFontMgr {
private:
    struct DataId {
        IUnknown* fLoader;  // In COM only IUnknown pointers may be safely used for identity.
        void* fKey;
        UINT32 fKeySize;

        DataId() { }

        DataId(DataId&& that) : fLoader(that.fLoader), fKey(that.fKey), fKeySize(that.fKeySize) {
            that.fLoader = nullptr;
            that.fKey = nullptr;
            SkDEBUGCODE(that.fKeySize = 0xFFFFFFFF;)
        }

        ~DataId() {
            if (fLoader) {
                fLoader->Release();
            }
            sk_free(fKey);
        }
    };

    mutable SkTArray<DataId> fDataIdCache;
    mutable SkMutex fDataIdCacheMutex;

    int FindOrAdd(IDWriteFontFileLoader* fontFileLoader,
                  const void* refKey, UINT32 refKeySize) const
    {
        SkTScopedComPtr<IUnknown> fontFileLoaderId;
        HR_GENERAL(fontFileLoader->QueryInterface(&fontFileLoaderId),
                   "Failed to re-convert to IDWriteFontFileLoader.",
                   SkFontIdentity::kInvalidDataId);

        SkAutoMutexAcquire ama(fDataIdCacheMutex);
        int count = fDataIdCache.count();
        int i;
        for (i = 0; i < count; ++i) {
            const DataId& current = fDataIdCache[i];
            if (fontFileLoaderId.get() == current.fLoader &&
                refKeySize == current.fKeySize &&
                0 == memcmp(refKey, current.fKey, refKeySize))
            {
                return i;
            }
        }
        DataId& added = fDataIdCache.push_back();
        added.fLoader = fontFileLoaderId.release();  // Ref is passed.
        added.fKey = sk_malloc_throw(refKeySize);
        memcpy(added.fKey, refKey, refKeySize);
        added.fKeySize = refKeySize;

        return i;
    }

public:


    /** localeNameLength must include the null terminator. */
    SkRemotableFontMgr_DirectWrite(IDWriteFontCollection* fontCollection,
                                   WCHAR* localeName, int localeNameLength)
        : fFontCollection(SkRefComPtr(fontCollection))
        , fLocaleName(localeNameLength)
    {
        memcpy(fLocaleName.get(), localeName, localeNameLength * sizeof(WCHAR));
    }

    HRESULT FontToIdentity(IDWriteFont* font, SkFontIdentity* fontId) const {
        SkTScopedComPtr<IDWriteFontFace> fontFace;
        HRM(font->CreateFontFace(&fontFace), "Could not create font face.");

        UINT32 numFiles;
        HR(fontFace->GetFiles(&numFiles, nullptr));
        if (numFiles > 1) {
            return E_FAIL;
        }

        // data id
        SkTScopedComPtr<IDWriteFontFile> fontFile;
        HR(fontFace->GetFiles(&numFiles, &fontFile));

        SkTScopedComPtr<IDWriteFontFileLoader> fontFileLoader;
        HR(fontFile->GetLoader(&fontFileLoader));

        const void* refKey;
        UINT32 refKeySize;
        HR(fontFile->GetReferenceKey(&refKey, &refKeySize));

        fontId->fDataId = FindOrAdd(fontFileLoader.get(), refKey, refKeySize);

        // index
        fontId->fTtcIndex = fontFace->GetIndex();

        // style
        fontId->fFontStyle = get_style(font);
        return S_OK;
    }

    SkRemotableFontIdentitySet* getIndex(int familyIndex) const override {
        SkTScopedComPtr<IDWriteFontFamily> fontFamily;
        HRNM(fFontCollection->GetFontFamily(familyIndex, &fontFamily),
             "Could not get requested family.");

        int count = fontFamily->GetFontCount();
        SkFontIdentity* fontIds;
        sk_sp<SkRemotableFontIdentitySet> fontIdSet(
            new SkRemotableFontIdentitySet(count, &fontIds));
        for (int fontIndex = 0; fontIndex < count; ++fontIndex) {
            SkTScopedComPtr<IDWriteFont> font;
            HRNM(fontFamily->GetFont(fontIndex, &font), "Could not get font.");

            HRN(FontToIdentity(font.get(), &fontIds[fontIndex]));
        }
        return fontIdSet.release();
    }

    virtual SkFontIdentity matchIndexStyle(int familyIndex,
                                           const SkFontStyle& pattern) const override
    {
        SkFontIdentity identity = { SkFontIdentity::kInvalidDataId };

        SkTScopedComPtr<IDWriteFontFamily> fontFamily;
        HR_GENERAL(fFontCollection->GetFontFamily(familyIndex, &fontFamily),
                   "Could not get requested family.",
                   identity);

        const DWriteStyle dwStyle(pattern);
        SkTScopedComPtr<IDWriteFont> font;
        HR_GENERAL(fontFamily->GetFirstMatchingFont(dwStyle.fWeight, dwStyle.fWidth,
                                                    dwStyle.fSlant, &font),
                   "Could not match font in family.",
                   identity);

        HR_GENERAL(FontToIdentity(font.get(), &identity), nullptr, identity);

        return identity;
    }

    static HRESULT getDefaultFontFamilyName(SkSMallocWCHAR* name) {
        NONCLIENTMETRICSW metrics;
        metrics.cbSize = sizeof(metrics);
        if (0 == SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,
                                       sizeof(metrics),
                                       &metrics,
                                       0)) {
            return E_UNEXPECTED;
        }

        size_t len = wcsnlen_s(metrics.lfMessageFont.lfFaceName, LF_FACESIZE) + 1;
        if (0 != wcsncpy_s(name->reset(len), len, metrics.lfMessageFont.lfFaceName, _TRUNCATE)) {
            return E_UNEXPECTED;
        }

        return S_OK;
    }

    SkRemotableFontIdentitySet* matchName(const char familyName[]) const override {
        SkSMallocWCHAR dwFamilyName;
        if (nullptr == familyName) {
            HR_GENERAL(getDefaultFontFamilyName(&dwFamilyName),
                       nullptr, SkRemotableFontIdentitySet::NewEmpty());
        } else {
            HR_GENERAL(sk_cstring_to_wchar(familyName, &dwFamilyName),
                       nullptr, SkRemotableFontIdentitySet::NewEmpty());
        }

        UINT32 index;
        BOOL exists;
        HR_GENERAL(fFontCollection->FindFamilyName(dwFamilyName.get(), &index, &exists),
                   "Failed while finding family by name.",
                   SkRemotableFontIdentitySet::NewEmpty());
        if (!exists) {
            return SkRemotableFontIdentitySet::NewEmpty();
        }

        return this->getIndex(index);
    }

    virtual SkFontIdentity matchNameStyle(const char familyName[],
                                          const SkFontStyle& style) const override
    {
        SkFontIdentity identity = { SkFontIdentity::kInvalidDataId };

        SkSMallocWCHAR dwFamilyName;
        if (nullptr == familyName) {
            HR_GENERAL(getDefaultFontFamilyName(&dwFamilyName), nullptr, identity);
        } else {
            HR_GENERAL(sk_cstring_to_wchar(familyName, &dwFamilyName), nullptr, identity);
        }

        UINT32 index;
        BOOL exists;
        HR_GENERAL(fFontCollection->FindFamilyName(dwFamilyName.get(), &index, &exists),
                   "Failed while finding family by name.",
                   identity);
        if (!exists) {
            return identity;
        }

        return this->matchIndexStyle(index, style);
    }

    class FontFallbackRenderer : public IDWriteTextRenderer {
    public:
        FontFallbackRenderer(const SkRemotableFontMgr_DirectWrite* outer, UINT32 character)
            : fRefCount(1), fOuter(SkSafeRef(outer)), fCharacter(character) {
          fIdentity.fDataId = SkFontIdentity::kInvalidDataId;
        }

        virtual ~FontFallbackRenderer() { }

        // IDWriteTextRenderer methods
        virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            DWRITE_GLYPH_RUN const* glyphRun,
            DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            IUnknown* clientDrawingEffect) override
        {
            SkTScopedComPtr<IDWriteFont> font;
            HRM(fOuter->fFontCollection->GetFontFromFontFace(glyphRun->fontFace, &font),
                "Could not get font from font face.");

            // It is possible that the font passed does not actually have the requested character,
            // due to no font being found and getting the fallback font.
            // Check that the font actually contains the requested character.
            BOOL exists;
            HRM(font->HasCharacter(fCharacter, &exists), "Could not find character.");

            if (exists) {
                HR(fOuter->FontToIdentity(font.get(), &fIdentity));
            }

            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE DrawUnderline(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_UNDERLINE const* underline,
            IUnknown* clientDrawingEffect) override
        { return E_NOTIMPL; }

        virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_STRIKETHROUGH const* strikethrough,
            IUnknown* clientDrawingEffect) override
        { return E_NOTIMPL; }

        virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(
            void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect) override
        { return E_NOTIMPL; }

        // IDWritePixelSnapping methods
        virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
            void* clientDrawingContext,
            BOOL* isDisabled) override
        {
            *isDisabled = FALSE;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(
            void* clientDrawingContext,
            DWRITE_MATRIX* transform) override
        {
            const DWRITE_MATRIX ident = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
            *transform = ident;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
            void* clientDrawingContext,
            FLOAT* pixelsPerDip) override
        {
            *pixelsPerDip = 1.0f;
            return S_OK;
        }

        // IUnknown methods
        ULONG STDMETHODCALLTYPE AddRef() override {
            return InterlockedIncrement(&fRefCount);
        }

        ULONG STDMETHODCALLTYPE Release() override {
            ULONG newCount = InterlockedDecrement(&fRefCount);
            if (0 == newCount) {
                delete this;
            }
            return newCount;
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            IID const& riid, void** ppvObject) override
        {
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

        const SkFontIdentity FallbackIdentity() { return fIdentity; }

    protected:
        ULONG fRefCount;
        sk_sp<const SkRemotableFontMgr_DirectWrite> fOuter;
        UINT32 fCharacter;
        SkFontIdentity fIdentity;
    };

    virtual SkFontIdentity matchNameStyleCharacter(const char familyName[],
                                                   const SkFontStyle& pattern,
                                                   const char* bcp47[], int bcp47Count,
                                                   SkUnichar character) const override
    {
        SkFontIdentity identity = { SkFontIdentity::kInvalidDataId };

        IDWriteFactory* dwFactory = sk_get_dwrite_factory();
        if (nullptr == dwFactory) {
            return identity;
        }

        // TODO: use IDWriteFactory2::GetSystemFontFallback when available.

        const DWriteStyle dwStyle(pattern);

        SkSMallocWCHAR dwFamilyName;
        if (nullptr == familyName) {
            HR_GENERAL(getDefaultFontFamilyName(&dwFamilyName), nullptr, identity);
        } else {
            HR_GENERAL(sk_cstring_to_wchar(familyName, &dwFamilyName), nullptr, identity);
        }

        const SkSMallocWCHAR* dwBcp47;
        SkSMallocWCHAR dwBcp47Local;
        if (bcp47Count < 1) {
            dwBcp47 = &fLocaleName;
        } else {
            //TODO: support fallback stack.
            HR_GENERAL(sk_cstring_to_wchar(bcp47[bcp47Count-1], &dwBcp47Local), nullptr, identity);
            dwBcp47 = &dwBcp47Local;
        }

        SkTScopedComPtr<IDWriteTextFormat> fallbackFormat;
        HR_GENERAL(dwFactory->CreateTextFormat(dwFamilyName,
                                               fFontCollection.get(),
                                               dwStyle.fWeight,
                                               dwStyle.fSlant,
                                               dwStyle.fWidth,
                                               72.0f,
                                               *dwBcp47,
                                               &fallbackFormat),
                   "Could not create text format.",
                   identity);

        WCHAR str[16];
        UINT32 strLen = static_cast<UINT32>(
            SkUTF::ToUTF16(character, reinterpret_cast<uint16_t*>(str)));
        SkTScopedComPtr<IDWriteTextLayout> fallbackLayout;
        HR_GENERAL(dwFactory->CreateTextLayout(str, strLen, fallbackFormat.get(),
                                               200.0f, 200.0f,
                                               &fallbackLayout),
                   "Could not create text layout.",
                   identity);

        SkTScopedComPtr<FontFallbackRenderer> fontFallbackRenderer(
            new FontFallbackRenderer(this, character));

        HR_GENERAL(fallbackLayout->Draw(nullptr, fontFallbackRenderer.get(), 50.0f, 50.0f),
                   "Could not draw layout with renderer.",
                   identity);

        return fontFallbackRenderer->FallbackIdentity();
    }

    SkStreamAsset* getData(int dataId) const override {
        SkAutoMutexAcquire ama(fDataIdCacheMutex);
        if (dataId >= fDataIdCache.count()) {
            return nullptr;
        }
        const DataId& id = fDataIdCache[dataId];

        SkTScopedComPtr<IDWriteFontFileLoader> loader;
        HRNM(id.fLoader->QueryInterface(&loader), "QuerryInterface IDWriteFontFileLoader failed");

        SkTScopedComPtr<IDWriteFontFileStream> fontFileStream;
        HRNM(loader->CreateStreamFromKey(id.fKey, id.fKeySize, &fontFileStream),
             "Could not create font file stream.");

        return new SkDWriteFontFileStream(fontFileStream.get());
    }

private:
    SkTScopedComPtr<IDWriteFontCollection> fFontCollection;
    SkSMallocWCHAR fLocaleName;

    typedef SkRemotableFontMgr INHERITED;
};

SkRemotableFontMgr* SkRemotableFontMgr_New_DirectWrite() {
    IDWriteFactory* factory = sk_get_dwrite_factory();
    if (nullptr == factory) {
        return nullptr;
    }

    SkTScopedComPtr<IDWriteFontCollection> sysFontCollection;
    HRNM(factory->GetSystemFontCollection(&sysFontCollection, FALSE),
         "Could not get system font collection.");

    WCHAR localeNameStorage[LOCALE_NAME_MAX_LENGTH];
    WCHAR* localeName = nullptr;
    int localeNameLen = 0;

    // Dynamically load GetUserDefaultLocaleName function, as it is not available on XP.
    SkGetUserDefaultLocaleNameProc getUserDefaultLocaleNameProc = nullptr;
    HRESULT hr = SkGetGetUserDefaultLocaleNameProc(&getUserDefaultLocaleNameProc);
    if (nullptr == getUserDefaultLocaleNameProc) {
        SK_TRACEHR(hr, "Could not get GetUserDefaultLocaleName.");
    } else {
        localeNameLen = getUserDefaultLocaleNameProc(localeNameStorage, LOCALE_NAME_MAX_LENGTH);
        if (localeNameLen) {
            localeName = localeNameStorage;
        };
    }

    return new SkRemotableFontMgr_DirectWrite(sysFontCollection.get(), localeName, localeNameLen);
}
#endif//defined(SK_BUILD_FOR_WIN)
