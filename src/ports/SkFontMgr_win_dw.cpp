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
#include "include/private/SkMutex.h"
#include "src/core/SkEndian.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkTypefaceCache.h"
#include "src/ports/SkTypeface_win_dw.h"
#include "src/utils/SkUTF.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkDWriteFontFileStream.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>
#include <dwrite_2.h>
#include <dwrite_3.h>

////////////////////////////////////////////////////////////////////////////////

class StreamFontFileLoader : public IDWriteFontFileLoader {
public:
    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileLoader methods
    virtual HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
        void const* fontFileReferenceKey,
        UINT32 fontFileReferenceKeySize,
        IDWriteFontFileStream** fontFileStream);

    // Takes ownership of stream.
    static HRESULT Create(std::unique_ptr<SkStreamAsset> stream,
                          StreamFontFileLoader** streamFontFileLoader) {
        *streamFontFileLoader = new StreamFontFileLoader(std::move(stream));
        if (nullptr == *streamFontFileLoader) {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    std::unique_ptr<SkStreamAsset> fStream;

private:
    StreamFontFileLoader(std::unique_ptr<SkStreamAsset> stream)
        : fStream(std::move(stream)), fRefCount(1)
    {}
    virtual ~StreamFontFileLoader() { }

    ULONG fRefCount;
};

HRESULT StreamFontFileLoader::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileLoader)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

ULONG StreamFontFileLoader::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

ULONG StreamFontFileLoader::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

HRESULT StreamFontFileLoader::CreateStreamFromKey(
    void const* fontFileReferenceKey,
    UINT32 fontFileReferenceKeySize,
    IDWriteFontFileStream** fontFileStream)
{
    SkTScopedComPtr<SkDWriteFontFileStreamWrapper> stream;
    HR(SkDWriteFontFileStreamWrapper::Create(fStream->duplicate().release(), &stream));
    *fontFileStream = stream.release();
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

class StreamFontFileEnumerator : public IDWriteFontFileEnumerator {
public:
    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileEnumerator methods
    virtual HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasCurrentFile);
    virtual HRESULT STDMETHODCALLTYPE GetCurrentFontFile(IDWriteFontFile** fontFile);

    static HRESULT Create(IDWriteFactory* factory, IDWriteFontFileLoader* fontFileLoader,
                          StreamFontFileEnumerator** streamFontFileEnumerator) {
        *streamFontFileEnumerator = new StreamFontFileEnumerator(factory, fontFileLoader);
        if (nullptr == *streamFontFileEnumerator) {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }
private:
    StreamFontFileEnumerator(IDWriteFactory* factory, IDWriteFontFileLoader* fontFileLoader);
    virtual ~StreamFontFileEnumerator() { }

    ULONG fRefCount;

    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFontFile> fCurrentFile;
    SkTScopedComPtr<IDWriteFontFileLoader> fFontFileLoader;
    bool fHasNext;
};

StreamFontFileEnumerator::StreamFontFileEnumerator(IDWriteFactory* factory,
                                                   IDWriteFontFileLoader* fontFileLoader)
    : fRefCount(1)
    , fFactory(SkRefComPtr(factory))
    , fCurrentFile()
    , fFontFileLoader(SkRefComPtr(fontFileLoader))
    , fHasNext(true)
{ }

HRESULT StreamFontFileEnumerator::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileEnumerator)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

ULONG StreamFontFileEnumerator::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

ULONG StreamFontFileEnumerator::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

HRESULT StreamFontFileEnumerator::MoveNext(BOOL* hasCurrentFile) {
    *hasCurrentFile = FALSE;

    if (!fHasNext) {
        return S_OK;
    }
    fHasNext = false;

    UINT32 dummy = 0;
    HR(fFactory->CreateCustomFontFileReference(
            &dummy, //cannot be nullptr
            sizeof(dummy), //even if this is 0
            fFontFileLoader.get(),
            &fCurrentFile));

    *hasCurrentFile = TRUE;
    return S_OK;
}

HRESULT StreamFontFileEnumerator::GetCurrentFontFile(IDWriteFontFile** fontFile) {
    if (fCurrentFile.get() == nullptr) {
        *fontFile = nullptr;
        return E_FAIL;
    }

    *fontFile = SkRefComPtr(fCurrentFile.get());
    return  S_OK;
}

////////////////////////////////////////////////////////////////////////////////

class StreamFontCollectionLoader : public IDWriteFontCollectionLoader {
public:
    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontCollectionLoader methods
    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
        IDWriteFactory* factory,
        void const* collectionKey,
        UINT32 collectionKeySize,
        IDWriteFontFileEnumerator** fontFileEnumerator);

    static HRESULT Create(IDWriteFontFileLoader* fontFileLoader,
                          StreamFontCollectionLoader** streamFontCollectionLoader) {
        *streamFontCollectionLoader = new StreamFontCollectionLoader(fontFileLoader);
        if (nullptr == *streamFontCollectionLoader) {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }
private:
    StreamFontCollectionLoader(IDWriteFontFileLoader* fontFileLoader)
        : fRefCount(1)
        , fFontFileLoader(SkRefComPtr(fontFileLoader))
    { }
    virtual ~StreamFontCollectionLoader() { }

    ULONG fRefCount;
    SkTScopedComPtr<IDWriteFontFileLoader> fFontFileLoader;
};

HRESULT StreamFontCollectionLoader::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontCollectionLoader)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

ULONG StreamFontCollectionLoader::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

ULONG StreamFontCollectionLoader::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

HRESULT StreamFontCollectionLoader::CreateEnumeratorFromKey(
    IDWriteFactory* factory,
    void const* collectionKey,
    UINT32 collectionKeySize,
    IDWriteFontFileEnumerator** fontFileEnumerator)
{
    SkTScopedComPtr<StreamFontFileEnumerator> enumerator;
    HR(StreamFontFileEnumerator::Create(factory, fFontFileLoader.get(), &enumerator));
    *fontFileEnumerator = enumerator.release();
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

class SkFontMgr_DirectWrite : public SkFontMgr {
public:
    /** localeNameLength and defaultFamilyNameLength must include the null terminator. */
    SkFontMgr_DirectWrite(IDWriteFactory* factory, IDWriteFontCollection* fontCollection,
                          IDWriteFontFallback* fallback,
                          WCHAR* localeName, int localeNameLength,
                          WCHAR* defaultFamilyName, int defaultFamilyNameLength)
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
    SkFontStyleSet* onCreateStyleSet(int index) const override;
    SkFontStyleSet* onMatchFamily(const char familyName[]) const override;
    SkTypeface* onMatchFamilyStyle(const char familyName[],
                                   const SkFontStyle& fontstyle) const override;
    SkTypeface* onMatchFamilyStyleCharacter(const char familyName[], const SkFontStyle&,
                                            const char* bcp47[], int bcp47Count,
                                            SkUnichar character) const override;
    SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                 const SkFontStyle& fontstyle) const override;
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
    SkTypeface* createTypeface(int index) override;
    SkTypeface* matchStyle(const SkFontStyle& pattern) override;

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
        face = DWriteFontTypeface::Make(fFactory.get(), fontFace, font, fontFamily);
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

SkFontStyleSet* SkFontMgr_DirectWrite::onCreateStyleSet(int index) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRNM(fFontCollection->GetFontFamily(index, &fontFamily), "Could not get requested family.");

    return new SkFontStyleSet_DirectWrite(this, fontFamily.get());
}

SkFontStyleSet* SkFontMgr_DirectWrite::onMatchFamily(const char familyName[]) const {
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

SkTypeface* SkFontMgr_DirectWrite::onMatchFamilyStyle(const char familyName[],
                                                      const SkFontStyle& fontstyle) const {
    sk_sp<SkFontStyleSet> sset(this->matchFamily(familyName));
    return sset->matchStyle(fontstyle);
}

class FontFallbackRenderer : public IDWriteTextRenderer {
public:
    FontFallbackRenderer(const SkFontMgr_DirectWrite* outer, UINT32 character)
        : fRefCount(1), fOuter(SkSafeRef(outer)), fCharacter(character), fResolvedTypeface(nullptr) {
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
        const DWRITE_MATRIX ident = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
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

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& riid, void** ppvObject) override{
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

    sk_sp<SkTypeface> ConsumeFallbackTypeface() { return std::move(fResolvedTypeface); }

protected:
    ULONG fRefCount;
    sk_sp<const SkFontMgr_DirectWrite> fOuter;
    UINT32 fCharacter;
    sk_sp<SkTypeface> fResolvedTypeface;
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

    virtual ~FontFallbackSource() { }

    // IDWriteTextAnalysisSource methods
    virtual HRESULT STDMETHODCALLTYPE GetTextAtPosition(
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

    virtual HRESULT STDMETHODCALLTYPE GetTextBeforePosition(
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

    virtual DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection() override {
        // TODO: this is also interesting.
        return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
    }

    virtual HRESULT STDMETHODCALLTYPE GetLocaleName(
        UINT32 textPosition,
        UINT32* textLength,
        WCHAR const** localeName) override
    {
        *localeName = fLocale;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE GetNumberSubstitution(
        UINT32 textPosition,
        UINT32* textLength,
        IDWriteNumberSubstitution** numberSubstitution) override
    {
        *numberSubstitution = fNumberSubstitution;
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

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& riid, void** ppvObject) override{
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

protected:
    ULONG fRefCount;
    const WCHAR* fString;
    UINT32 fLength;
    const WCHAR* fLocale;
    IDWriteNumberSubstitution* fNumberSubstitution;
};

SkTypeface* SkFontMgr_DirectWrite::onMatchFamilyStyleCharacter(const char familyName[],
                                                               const SkFontStyle& style,
                                                               const char* bcp47[], int bcp47Count,
                                                               SkUnichar character) const
{
    const DWriteStyle dwStyle(style);

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
        return this->fallback(dwFamilyName, dwStyle, dwBcp47->get(), character).release();
    }

    // LayoutFallback may use the system font collection for fallback.
    return this->layoutFallback(dwFamilyName, dwStyle, dwBcp47->get(), character).release();
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::fallback(const WCHAR* dwFamilyName,
                                                  DWriteStyle dwStyle,
                                                  const WCHAR* dwBcp47,
                                                  UINT32 character) const
{
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
    HRNM(fFontFallback->MapCharacters(fontFallbackSource.get(),
                                      0, // textPosition,
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
    HRNM(fFactory->CreateTextLayout(str, strLen, fallbackFormat.get(),
                                    200.0f, 200.0f,
                                    &fallbackLayout),
         "Could not create text layout.");

    SkTScopedComPtr<FontFallbackRenderer> fontFallbackRenderer(
        new FontFallbackRenderer(this, character));

    HRNM(fallbackLayout->SetFontCollection(fFontCollection.get(), { 0, strLen }),
         "Could not set layout font collection.");
    HRNM(fallbackLayout->Draw(nullptr, fontFallbackRenderer.get(), 50.0f, 50.0f),
         "Could not draw layout with renderer.");

    return fontFallbackRenderer->ConsumeFallbackTypeface();
}

SkTypeface* SkFontMgr_DirectWrite::onMatchFaceStyle(const SkTypeface* familyMember,
                                                    const SkFontStyle& fontstyle) const {
    SkString familyName;
    SkFontStyleSet_DirectWrite sset(
        this, ((DWriteFontTypeface*)familyMember)->fDWriteFontFamily.get()
    );
    return sset.matchStyle(fontstyle);
}

template <typename T> class SkAutoIDWriteUnregister {
public:
    SkAutoIDWriteUnregister(IDWriteFactory* factory, T* unregister)
        : fFactory(factory), fUnregister(unregister)
    { }

    ~SkAutoIDWriteUnregister() {
        if (fUnregister) {
            unregister(fFactory, fUnregister);
        }
    }

    T* detatch() {
        T* old = fUnregister;
        fUnregister = nullptr;
        return old;
    }

private:
    HRESULT unregister(IDWriteFactory* factory, IDWriteFontFileLoader* unregister) {
        return factory->UnregisterFontFileLoader(unregister);
    }

    HRESULT unregister(IDWriteFactory* factory, IDWriteFontCollectionLoader* unregister) {
        return factory->UnregisterFontCollectionLoader(unregister);
    }

    IDWriteFactory* fFactory;
    T* fUnregister;
};

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                                               int ttcIndex) const {
    SkTScopedComPtr<StreamFontFileLoader> fontFileLoader;
    // This transfers ownership of stream to the new object.
    HRN(StreamFontFileLoader::Create(std::move(stream), &fontFileLoader));
    HRN(fFactory->RegisterFontFileLoader(fontFileLoader.get()));
    SkAutoIDWriteUnregister<StreamFontFileLoader> autoUnregisterFontFileLoader(
        fFactory.get(), fontFileLoader.get());

    SkTScopedComPtr<StreamFontCollectionLoader> fontCollectionLoader;
    HRN(StreamFontCollectionLoader::Create(fontFileLoader.get(), &fontCollectionLoader));
    HRN(fFactory->RegisterFontCollectionLoader(fontCollectionLoader.get()));
    SkAutoIDWriteUnregister<StreamFontCollectionLoader> autoUnregisterFontCollectionLoader(
        fFactory.get(), fontCollectionLoader.get());

    SkTScopedComPtr<IDWriteFontCollection> fontCollection;
    HRN(fFactory->CreateCustomFontCollection(fontCollectionLoader.get(), nullptr, 0, &fontCollection));

    // Find the first non-simulated font which has the given ttc index.
    UINT32 familyCount = fontCollection->GetFontFamilyCount();
    for (UINT32 familyIndex = 0; familyIndex < familyCount; ++familyIndex) {
        SkTScopedComPtr<IDWriteFontFamily> fontFamily;
        HRN(fontCollection->GetFontFamily(familyIndex, &fontFamily));

        UINT32 fontCount = fontFamily->GetFontCount();
        for (UINT32 fontIndex = 0; fontIndex < fontCount; ++fontIndex) {
            SkTScopedComPtr<IDWriteFont> font;
            HRN(fontFamily->GetFont(fontIndex, &font));
            if (font->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE) {
                continue;
            }

            SkTScopedComPtr<IDWriteFontFace> fontFace;
            HRN(font->CreateFontFace(&fontFace));

            int faceIndex = fontFace->GetIndex();
            if (faceIndex == ttcIndex) {
                return DWriteFontTypeface::Make(fFactory.get(),
                                                fontFace.get(), font.get(), fontFamily.get(),
                                                autoUnregisterFontFileLoader.detatch(),
                                                autoUnregisterFontCollectionLoader.detatch());
            }
        }
    }

    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                                              const SkFontArguments& args) const {
    SkTScopedComPtr<StreamFontFileLoader> fontFileLoader;
    // This transfers ownership of stream to the new object.
    HRN(StreamFontFileLoader::Create(std::move(stream), &fontFileLoader));
    HRN(fFactory->RegisterFontFileLoader(fontFileLoader.get()));
    SkAutoIDWriteUnregister<StreamFontFileLoader> autoUnregisterFontFileLoader(
            fFactory.get(), fontFileLoader.get());

    SkTScopedComPtr<StreamFontCollectionLoader> fontCollectionLoader;
    HRN(StreamFontCollectionLoader::Create(fontFileLoader.get(), &fontCollectionLoader));
    HRN(fFactory->RegisterFontCollectionLoader(fontCollectionLoader.get()));
    SkAutoIDWriteUnregister<StreamFontCollectionLoader> autoUnregisterFontCollectionLoader(
            fFactory.get(), fontCollectionLoader.get());

    SkTScopedComPtr<IDWriteFontCollection> fontCollection;
    HRN(fFactory->CreateCustomFontCollection(fontCollectionLoader.get(), nullptr, 0,
                                             &fontCollection));

    // Find the first non-simulated font which has the given ttc index.
    UINT32 familyCount = fontCollection->GetFontFamilyCount();
    for (UINT32 familyIndex = 0; familyIndex < familyCount; ++familyIndex) {
        SkTScopedComPtr<IDWriteFontFamily> fontFamily;
        HRN(fontCollection->GetFontFamily(familyIndex, &fontFamily));

        UINT32 fontCount = fontFamily->GetFontCount();
        for (UINT32 fontIndex = 0; fontIndex < fontCount; ++fontIndex) {
            SkTScopedComPtr<IDWriteFont> font;
            HRN(fontFamily->GetFont(fontIndex, &font));

            // Skip if the current font is simulated
            if (font->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE) {
                continue;
            }
            SkTScopedComPtr<IDWriteFontFace> fontFace;
            HRN(font->CreateFontFace(&fontFace));
            int faceIndex = fontFace->GetIndex();
            int ttcIndex = args.getCollectionIndex();

            // Skip if the current face index does not match the ttcIndex
            if (faceIndex != ttcIndex) {
                continue;
            }

#if defined(NTDDI_WIN10_RS3) && NTDDI_VERSION >= NTDDI_WIN10_RS3

            SkTScopedComPtr<IDWriteFontFace5> fontFace5;
            if (SUCCEEDED(fontFace->QueryInterface(&fontFace5)) && fontFace5->HasVariations()) {
                UINT32 fontAxisCount = fontFace5->GetFontAxisValueCount();
                UINT32 argsCoordCount = args.getVariationDesignPosition().coordinateCount;
                SkAutoSTMalloc<8, DWRITE_FONT_AXIS_VALUE> fontAxisValues(fontAxisCount);
                SkTScopedComPtr<IDWriteFontResource> fontResource;
                HRN(fontFace5->GetFontResource(&fontResource));
                // Set all axes by default values
                HRN(fontResource->GetDefaultFontAxisValues(fontAxisValues, fontAxisCount));

                for (UINT32 fontIndex = 0; fontIndex < fontAxisCount; ++fontIndex) {
                    for (UINT32 argsIndex = 0; argsIndex < argsCoordCount; ++argsIndex) {
                        if (SkEndian_SwapBE32(fontAxisValues[fontIndex].axisTag) ==
                            args.getVariationDesignPosition().coordinates[argsIndex].axis) {
                            fontAxisValues[fontIndex].value =
                                args.getVariationDesignPosition().coordinates[argsIndex].value;
                        }
                    }
                }

                SkTScopedComPtr<IDWriteFontFace5> fontFace5_Out;
                HRN(fontResource->CreateFontFace(DWRITE_FONT_SIMULATIONS_NONE,
                                                 fontAxisValues.get(),
                                                 fontAxisCount,
                                                 &fontFace5_Out));
                fontFace.reset();
                HRN(fontFace5_Out->QueryInterface(&fontFace));
            }

#endif

            return DWriteFontTypeface::Make(
                    fFactory.get(), fontFace.get(), font.get(), fontFamily.get(),
                    autoUnregisterFontFileLoader.detatch(),
                    autoUnregisterFontCollectionLoader.detatch());
        }
    }

    return nullptr;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onMakeFromData(sk_sp<SkData> data, int ttcIndex) const {
    return this->makeFromStream(skstd::make_unique<SkMemoryStream>(std::move(data)), ttcIndex);
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
    const DWriteStyle dwStyle(style);
    if (familyName) {
        SkSMallocWCHAR dwFamilyName;
        if (SUCCEEDED(sk_cstring_to_wchar(familyName, &dwFamilyName))) {
            this->getByFamilyName(dwFamilyName, &fontFamily);
            if (!fontFamily && fFontFallback) {
                return this->fallback(dwFamilyName, dwStyle, fLocaleName.get(), 32);
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
    HRNM(fontFamily->GetFirstMatchingFont(dwStyle.fWeight, dwStyle.fWidth, dwStyle.fSlant, &font),
         "Could not get matching font.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return this->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get());
}

///////////////////////////////////////////////////////////////////////////////

int SkFontStyleSet_DirectWrite::count() {
    return fFontFamily->GetFontCount();
}

SkTypeface* SkFontStyleSet_DirectWrite::createTypeface(int index) {
    SkTScopedComPtr<IDWriteFont> font;
    HRNM(fFontFamily->GetFont(index, &font), "Could not get font.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return fFontMgr->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fFontFamily.get()).release();
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

SkTypeface* SkFontStyleSet_DirectWrite::matchStyle(const SkFontStyle& pattern) {
    SkTScopedComPtr<IDWriteFont> font;
    DWriteStyle dwStyle(pattern);
    // TODO: perhaps use GetMatchingFonts and get the least simulated?
    HRNM(fFontFamily->GetFirstMatchingFont(dwStyle.fWeight, dwStyle.fWidth, dwStyle.fSlant, &font),
         "Could not match font in family.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return fFontMgr->makeTypefaceFromDWriteFont(fontFace.get(), font.get(),
                                                fFontFamily.get()).release();
}

////////////////////////////////////////////////////////////////////////////////
#include "include/ports/SkTypeface_win.h"

SK_API sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
                                                  IDWriteFontCollection* collection) {
    return SkFontMgr_New_DirectWrite(factory, collection, nullptr);
}

SK_API sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
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

    WCHAR* defaultFamilyName = nullptr;
    int defaultFamilyNameLen = 0;
    NONCLIENTMETRICSW metrics;
    metrics.cbSize = sizeof(metrics);
    if (nullptr == fallback) {
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0)) {
            defaultFamilyName = metrics.lfMessageFont.lfFaceName;
            defaultFamilyNameLen = LF_FACESIZE;
        }
    }

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
        }
    }

    return sk_make_sp<SkFontMgr_DirectWrite>(factory, collection, fallback,
                                             localeName, localeNameLen,
                                             defaultFamilyName, defaultFamilyNameLen);
}

#include "include/ports/SkFontMgr_indirect.h"
SK_API sk_sp<SkFontMgr> SkFontMgr_New_DirectWriteRenderer(sk_sp<SkRemotableFontMgr> proxy) {
    sk_sp<SkFontMgr> impl(SkFontMgr_New_DirectWrite());
    if (!impl) {
        return nullptr;
    }
    return sk_make_sp<SkFontMgr_Indirect>(std::move(impl), std::move(proxy));
}
#endif//defined(SK_BUILD_FOR_WIN)
