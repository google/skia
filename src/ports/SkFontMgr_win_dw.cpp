/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "SkDWrite.h"
#include "SkDWriteFontFileStream.h"
#include "SkFontMgr.h"
#include "SkHRESULT.h"
#include "SkMakeUnique.h"
#include "SkMutex.h"
#include "SkStream.h"
#include "SkTScopedComPtr.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkTypeface_win_dw.h"
#include "SkTypes.h"
#include "SkUtils.h"

#include <dwrite.h>
#include <dwrite_2.h>

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
    /** localeNameLength must include the null terminator. */
    SkFontMgr_DirectWrite(IDWriteFactory* factory, IDWriteFontCollection* fontCollection,
                          IDWriteFontFallback* fallback, WCHAR* localeName, int localeNameLength)
        : fFactory(SkRefComPtr(factory))
        , fFontFallback(SkSafeRefComPtr(fallback))
        , fFontCollection(SkRefComPtr(fontCollection))
        , fLocaleName(localeNameLength)
    {
        if (!SUCCEEDED(fFactory->QueryInterface(&fFactory2))) {
            // IUnknown::QueryInterface states that if it fails, punk will be set to nullptr.
            // http://blogs.msdn.com/b/oldnewthing/archive/2004/03/26/96777.aspx
            SkASSERT_RELEASE(nullptr == fFactory2.get());
        }
        if (fFontFallback.get()) {
            // factory must be provided if fallback is non-null, else the fallback will not be used.
            SkASSERT(fFactory2.get());
        }
        memcpy(fLocaleName.get(), localeName, localeNameLength * sizeof(WCHAR));
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
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override;
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override;
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[], SkFontStyle) const override;

private:
    HRESULT getByFamilyName(const WCHAR familyName[], IDWriteFontFamily** fontFamily) const;
    HRESULT getDefaultFontFamily(IDWriteFontFamily** fontFamily) const;

    /** Creates a typeface using a typeface cache. */
    sk_sp<SkTypeface> makeTypefaceFromDWriteFont(IDWriteFontFace* fontFace,
                                                 IDWriteFont* font,
                                                 IDWriteFontFamily* fontFamily) const;

    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFactory2> fFactory2;
    SkTScopedComPtr<IDWriteFontFallback> fFontFallback;
    SkTScopedComPtr<IDWriteFontCollection> fFontCollection;
    SkSMallocWCHAR fLocaleName;
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
    SkAutoMutexAcquire ama(fTFCacheMutex);
    ProtoDWriteTypeface spec = { fontFace, font, fontFamily };
    SkTypeface* face = fTFCache.findByProcAndRef(FindByDWriteFont, &spec);
    if (nullptr == face) {
        face = DWriteFontTypeface::Create(fFactory.get(), fontFace, font, fontFamily);
        if (face) {
            fTFCache.add(face);
        }
    }
    return sk_sp<SkTypeface>(face);
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
                                                                   fontFamily.get()).release();
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

    SkTypeface* FallbackTypeface() { return fResolvedTypeface; }

protected:
    ULONG fRefCount;
    sk_sp<const SkFontMgr_DirectWrite> fOuter;
    UINT32 fCharacter;
    SkTypeface* fResolvedTypeface;
};

class FontFallbackSource : public IDWriteTextAnalysisSource {
public:
    FontFallbackSource(const WCHAR* string, UINT32 length, const WCHAR* locale,
                       IDWriteNumberSubstitution* numberSubstitution)
        : fString(string)
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

    WCHAR str[16];
    UINT32 strLen = static_cast<UINT32>(
        SkUTF16_FromUnichar(character, reinterpret_cast<uint16_t*>(str)));

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

    if (fFactory2.get()) {
        SkTScopedComPtr<IDWriteFontFallback> systemFontFallback;
        IDWriteFontFallback* fontFallback = fFontFallback.get();
        if (!fontFallback) {
            HRNM(fFactory2->GetSystemFontFallback(&systemFontFallback),
                 "Could not get system fallback.");
            fontFallback = systemFontFallback.get();
        }

        SkTScopedComPtr<IDWriteNumberSubstitution> numberSubstitution;
        HRNM(fFactory2->CreateNumberSubstitution(DWRITE_NUMBER_SUBSTITUTION_METHOD_NONE, nullptr, TRUE,
                                                 &numberSubstitution),
             "Could not create number substitution.");
        SkTScopedComPtr<FontFallbackSource> fontFallbackSource(
            new FontFallbackSource(str, strLen, *dwBcp47, numberSubstitution.get()));

        UINT32 mappedLength;
        SkTScopedComPtr<IDWriteFont> font;
        FLOAT scale;
        HRNM(fontFallback->MapCharacters(fontFallbackSource.get(),
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
        return this->makeTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get()).release();
    }

    SkTScopedComPtr<IDWriteTextFormat> fallbackFormat;
    HRNM(fFactory->CreateTextFormat(dwFamilyName ? dwFamilyName : L"",
                                    fFontCollection.get(),
                                    dwStyle.fWeight,
                                    dwStyle.fSlant,
                                    dwStyle.fWidth,
                                    72.0f,
                                    *dwBcp47,
                                    &fallbackFormat),
         "Could not create text format.");

    SkTScopedComPtr<IDWriteTextLayout> fallbackLayout;
    HRNM(fFactory->CreateTextLayout(str, strLen, fallbackFormat.get(),
                                    200.0f, 200.0f,
                                    &fallbackLayout),
         "Could not create text layout.");

    SkTScopedComPtr<FontFallbackRenderer> fontFallbackRenderer(
        new FontFallbackRenderer(this, character));

    HRNM(fallbackLayout->Draw(nullptr, fontFallbackRenderer.get(), 50.0f, 50.0f),
         "Could not draw layout with renderer.");

    return fontFallbackRenderer->FallbackTypeface();
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
                return sk_sp<SkTypeface>(DWriteFontTypeface::Create(fFactory.get(),
                                                  fontFace.get(), font.get(), fontFamily.get(),
                                                  autoUnregisterFontFileLoader.detatch(),
                                                  autoUnregisterFontCollectionLoader.detatch()));
            }
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

HRESULT SkFontMgr_DirectWrite::getDefaultFontFamily(IDWriteFontFamily** fontFamily) const {
    NONCLIENTMETRICSW metrics;
    metrics.cbSize = sizeof(metrics);
    if (0 == SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0)) {
        return E_UNEXPECTED;
    }
    HRM(this->getByFamilyName(metrics.lfMessageFont.lfFaceName, fontFamily),
        "Could not create DWrite font family from LOGFONT.");
    return S_OK;
}

sk_sp<SkTypeface> SkFontMgr_DirectWrite::onLegacyMakeTypeface(const char familyName[],
                                                              SkFontStyle style) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    if (familyName) {
        SkSMallocWCHAR wideFamilyName;
        if (SUCCEEDED(sk_cstring_to_wchar(familyName, &wideFamilyName))) {
            this->getByFamilyName(wideFamilyName, &fontFamily);
        }
    }

    if (nullptr == fontFamily.get()) {
        // No family with given name, try default.
        this->getDefaultFontFamily(&fontFamily);
    }

    if (nullptr == fontFamily.get()) {
        // Could not obtain the default font.
        HRNM(fFontCollection->GetFontFamily(0, &fontFamily),
             "Could not get default-default font family.");
    }

    SkTScopedComPtr<IDWriteFont> font;
    DWriteStyle dwStyle(style);
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
#include "SkTypeface_win.h"

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

    return sk_make_sp<SkFontMgr_DirectWrite>(factory, collection, fallback,
                                             localeName, localeNameLen);
}

#include "SkFontMgr_indirect.h"
SK_API sk_sp<SkFontMgr> SkFontMgr_New_DirectWriteRenderer(sk_sp<SkRemotableFontMgr> proxy) {
    sk_sp<SkFontMgr> impl(SkFontMgr_New_DirectWrite());
    if (!impl) {
        return nullptr;
    }
    return sk_make_sp<SkFontMgr_Indirect>(std::move(impl), std::move(proxy));
}
#endif//defined(SK_BUILD_FOR_WIN)
