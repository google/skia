/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDWrite.h"
#include "SkDWriteFontFileStream.h"
#include "SkFontMgr.h"
#include "SkHRESULT.h"
#include "SkStream.h"
#include "SkTScopedComPtr.h"
#include "SkThread.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkTypeface_win_dw.h"
#include "SkTypes.h"

#include <dwrite.h>

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

    static HRESULT Create(SkStream* stream, StreamFontFileLoader** streamFontFileLoader) {
        *streamFontFileLoader = new StreamFontFileLoader(stream);
        if (NULL == streamFontFileLoader) {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    SkAutoTUnref<SkStream> fStream;

private:
    StreamFontFileLoader(SkStream* stream) : fRefCount(1), fStream(SkRef(stream)) { }
    virtual ~StreamFontFileLoader() { }

    ULONG fRefCount;
};

HRESULT StreamFontFileLoader::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileLoader)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = NULL;
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
    HR(SkDWriteFontFileStreamWrapper::Create(fStream, &stream));
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
        if (NULL == streamFontFileEnumerator) {
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
        *ppvObject = NULL;
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
            &dummy, //cannot be NULL
            sizeof(dummy), //even if this is 0
            fFontFileLoader.get(),
            &fCurrentFile));

    *hasCurrentFile = TRUE;
    return S_OK;
}

HRESULT StreamFontFileEnumerator::GetCurrentFontFile(IDWriteFontFile** fontFile) {
    if (fCurrentFile.get() == NULL) {
        *fontFile = NULL;
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
        if (NULL == streamFontCollectionLoader) {
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
        *ppvObject = NULL;
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
                          WCHAR* localeName, int localeNameLength)
        : fFactory(SkRefComPtr(factory))
        , fFontCollection(SkRefComPtr(fontCollection))
        , fLocaleName(localeNameLength)
    {
        memcpy(fLocaleName.get(), localeName, localeNameLength * sizeof(WCHAR));
    }

protected:
    virtual int onCountFamilies() const SK_OVERRIDE;
    virtual void onGetFamilyName(int index, SkString* familyName) const SK_OVERRIDE;
    virtual SkFontStyleSet* onCreateStyleSet(int index) const SK_OVERRIDE;
    virtual SkFontStyleSet* onMatchFamily(const char familyName[]) const SK_OVERRIDE;
    virtual SkTypeface* onMatchFamilyStyle(const char familyName[],
                                           const SkFontStyle& fontstyle) const SK_OVERRIDE;
    virtual SkTypeface* onMatchFaceStyle(const SkTypeface* familyMember,
                                         const SkFontStyle& fontstyle) const SK_OVERRIDE;
    virtual SkTypeface* onCreateFromStream(SkStream* stream, int ttcIndex) const SK_OVERRIDE;
    virtual SkTypeface* onCreateFromData(SkData* data, int ttcIndex) const SK_OVERRIDE;
    virtual SkTypeface* onCreateFromFile(const char path[], int ttcIndex) const SK_OVERRIDE;
    virtual SkTypeface* onLegacyCreateTypeface(const char familyName[],
                                               unsigned styleBits) const SK_OVERRIDE;

private:
    HRESULT getByFamilyName(const WCHAR familyName[], IDWriteFontFamily** fontFamily) const;
    HRESULT getDefaultFontFamily(IDWriteFontFamily** fontFamily) const;

    /** Creates a typeface using a typeface cache. */
    SkTypeface* createTypefaceFromDWriteFont(IDWriteFontFace* fontFace,
                                             IDWriteFont* font,
                                             IDWriteFontFamily* fontFamily) const;

    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFontCollection> fFontCollection;
    SkSMallocWCHAR fLocaleName;
    mutable SkMutex fTFCacheMutex;
    mutable SkTypefaceCache fTFCache;

    friend class SkFontStyleSet_DirectWrite;
};

class SkFontStyleSet_DirectWrite : public SkFontStyleSet {
public:
    SkFontStyleSet_DirectWrite(const SkFontMgr_DirectWrite* fontMgr,
                               IDWriteFontFamily* fontFamily)
        : fFontMgr(SkRef(fontMgr))
        , fFontFamily(SkRefComPtr(fontFamily))
    { }

    virtual int count() SK_OVERRIDE;
    virtual void getStyle(int index, SkFontStyle* fs, SkString* styleName) SK_OVERRIDE;
    virtual SkTypeface* createTypeface(int index) SK_OVERRIDE;
    virtual SkTypeface* matchStyle(const SkFontStyle& pattern) SK_OVERRIDE;

private:
    SkAutoTUnref<const SkFontMgr_DirectWrite> fFontMgr;
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

static bool FindByDWriteFont(SkTypeface* cached, SkTypeface::Style, void* ctx) {
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
    HRB(cshFace->fDWriteFontFace->GetFiles(&cshNumFiles, NULL));
    HRB(ctxFace->fDWriteFontFace->GetFiles(&ctxNumFiles, NULL));
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

SkTypeface* SkFontMgr_DirectWrite::createTypefaceFromDWriteFont(
        IDWriteFontFace* fontFace,
        IDWriteFont* font,
        IDWriteFontFamily* fontFamily) const {
    SkAutoMutexAcquire ama(fTFCacheMutex);
    ProtoDWriteTypeface spec = { fontFace, font, fontFamily };
    SkTypeface* face = fTFCache.findByProcAndRef(FindByDWriteFont, &spec);
    if (NULL == face) {
        face = DWriteFontTypeface::Create(fFactory.get(), fontFace, font, fontFamily);
        if (face) {
            fTFCache.add(face, get_style(font), true);
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

    return SkNEW_ARGS(SkFontStyleSet_DirectWrite, (this, fontFamily.get()));
}

SkFontStyleSet* SkFontMgr_DirectWrite::onMatchFamily(const char familyName[]) const {
    SkSMallocWCHAR dwFamilyName;
    HRN(sk_cstring_to_wchar(familyName, &dwFamilyName));

    UINT32 index;
    BOOL exists;
    HRNM(fFontCollection->FindFamilyName(dwFamilyName.get(), &index, &exists),
            "Failed while finding family by name.");
    if (!exists) {
        return NULL;
    }

    return this->onCreateStyleSet(index);
}

SkTypeface* SkFontMgr_DirectWrite::onMatchFamilyStyle(const char familyName[],
                                                      const SkFontStyle& fontstyle) const {
    SkAutoTUnref<SkFontStyleSet> sset(this->matchFamily(familyName));
    return sset->matchStyle(fontstyle);
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
        fUnregister = NULL;
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

SkTypeface* SkFontMgr_DirectWrite::onCreateFromStream(SkStream* stream, int ttcIndex) const {
    SkTScopedComPtr<StreamFontFileLoader> fontFileLoader;
    HRN(StreamFontFileLoader::Create(stream, &fontFileLoader));
    HRN(fFactory->RegisterFontFileLoader(fontFileLoader.get()));
    SkAutoIDWriteUnregister<StreamFontFileLoader> autoUnregisterFontFileLoader(
        fFactory.get(), fontFileLoader.get());

    SkTScopedComPtr<StreamFontCollectionLoader> fontCollectionLoader;
    HRN(StreamFontCollectionLoader::Create(fontFileLoader.get(), &fontCollectionLoader));
    HRN(fFactory->RegisterFontCollectionLoader(fontCollectionLoader.get()));
    SkAutoIDWriteUnregister<StreamFontCollectionLoader> autoUnregisterFontCollectionLoader(
        fFactory.get(), fontCollectionLoader.get());

    SkTScopedComPtr<IDWriteFontCollection> fontCollection;
    HRN(fFactory->CreateCustomFontCollection(fontCollectionLoader.get(), NULL, 0, &fontCollection));

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

            UINT32 faceIndex = fontFace->GetIndex();
            if (faceIndex == ttcIndex) {
                return DWriteFontTypeface::Create(fFactory.get(),
                                                  fontFace.get(), font.get(), fontFamily.get(),
                                                  autoUnregisterFontFileLoader.detatch(),
                                                  autoUnregisterFontCollectionLoader.detatch());
            }
        }
    }

    return NULL;
}

SkTypeface* SkFontMgr_DirectWrite::onCreateFromData(SkData* data, int ttcIndex) const {
    SkAutoTUnref<SkStream> stream(SkNEW_ARGS(SkMemoryStream, (data)));
    return this->createFromStream(stream, ttcIndex);
}

SkTypeface* SkFontMgr_DirectWrite::onCreateFromFile(const char path[], int ttcIndex) const {
    SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(path));
    return this->createFromStream(stream, ttcIndex);
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
    if (0 == SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,
                                   sizeof(metrics),
                                   &metrics,
                                   0)) {
        return E_UNEXPECTED;
    }
    HRM(this->getByFamilyName(metrics.lfMessageFont.lfFaceName, fontFamily),
        "Could not create DWrite font family from LOGFONT.");
    return S_OK;
}

SkTypeface* SkFontMgr_DirectWrite::onLegacyCreateTypeface(const char familyName[],
                                                          unsigned styleBits) const {
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    if (familyName) {
        SkSMallocWCHAR wideFamilyName;
        if (SUCCEEDED(sk_cstring_to_wchar(familyName, &wideFamilyName))) {
            this->getByFamilyName(wideFamilyName, &fontFamily);
        }
    }

    if (NULL == fontFamily.get()) {
        // No family with given name, try default.
        HRNM(this->getDefaultFontFamily(&fontFamily), "Could not get default font family.");
    }

    if (NULL == fontFamily.get()) {
        // Could not obtain the default font.
        HRNM(fFontCollection->GetFontFamily(0, &fontFamily),
             "Could not get default-default font family.");
    }

    SkTScopedComPtr<IDWriteFont> font;
    DWRITE_FONT_WEIGHT weight = (styleBits & SkTypeface::kBold)
                              ? DWRITE_FONT_WEIGHT_BOLD
                              : DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
    DWRITE_FONT_STYLE italic = (styleBits & SkTypeface::kItalic)
                             ? DWRITE_FONT_STYLE_ITALIC
                             : DWRITE_FONT_STYLE_NORMAL;
    HRNM(fontFamily->GetFirstMatchingFont(weight, stretch, italic, &font),
         "Could not get matching font.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return this->createTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get());
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

    return fFontMgr->createTypefaceFromDWriteFont(fontFace.get(), font.get(), fFontFamily.get());
}

void SkFontStyleSet_DirectWrite::getStyle(int index, SkFontStyle* fs, SkString* styleName) {
    SkTScopedComPtr<IDWriteFont> font;
    HRVM(fFontFamily->GetFont(index, &font), "Could not get font.");

    if (fs) {
        SkFontStyle::Slant slant;
        switch (font->GetStyle()) {
        case DWRITE_FONT_STYLE_NORMAL:
            slant = SkFontStyle::kUpright_Slant;
            break;
        case DWRITE_FONT_STYLE_OBLIQUE:
        case DWRITE_FONT_STYLE_ITALIC:
            slant = SkFontStyle::kItalic_Slant;
            break;
        default:
            SkASSERT(false);
        }

        int weight = font->GetWeight();
        int width = font->GetStretch();

        *fs = SkFontStyle(weight, width, slant);
    }

    if (styleName) {
        SkTScopedComPtr<IDWriteLocalizedStrings> faceNames;
        if (SUCCEEDED(font->GetFaceNames(&faceNames))) {
            sk_get_locale_string(faceNames.get(), fFontMgr->fLocaleName.get(), styleName);
        }
    }
}

SkTypeface* SkFontStyleSet_DirectWrite::matchStyle(const SkFontStyle& pattern) {
    DWRITE_FONT_STYLE slant;
    switch (pattern.slant()) {
    case SkFontStyle::kUpright_Slant:
        slant = DWRITE_FONT_STYLE_NORMAL;
        break;
    case SkFontStyle::kItalic_Slant:
        slant = DWRITE_FONT_STYLE_ITALIC;
        break;
    default:
        SkASSERT(false);
    }

    DWRITE_FONT_WEIGHT weight = (DWRITE_FONT_WEIGHT)pattern.weight();
    DWRITE_FONT_STRETCH width = (DWRITE_FONT_STRETCH)pattern.width();

    SkTScopedComPtr<IDWriteFont> font;
    // TODO: perhaps use GetMatchingFonts and get the least simulated?
    HRNM(fFontFamily->GetFirstMatchingFont(weight, width, slant, &font),
         "Could not match font in family.");

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRNM(font->CreateFontFace(&fontFace), "Could not create font face.");

    return fFontMgr->createTypefaceFromDWriteFont(fontFace.get(), font.get(),
                                                  fFontFamily.get());
}

////////////////////////////////////////////////////////////////////////////////
#include "SkTypeface_win.h"

SK_API SkFontMgr* SkFontMgr_New_DirectWrite(IDWriteFactory* factory) {
    if (NULL == factory) {
        factory = sk_get_dwrite_factory();
        if (NULL == factory) {
            return NULL;
        }
    }

    SkTScopedComPtr<IDWriteFontCollection> sysFontCollection;
    HRNM(factory->GetSystemFontCollection(&sysFontCollection, FALSE),
         "Could not get system font collection.");

    WCHAR localeNameStorage[LOCALE_NAME_MAX_LENGTH];
    WCHAR* localeName = NULL;
    int localeNameLen = 0;

    // Dynamically load GetUserDefaultLocaleName function, as it is not available on XP.
    SkGetUserDefaultLocaleNameProc getUserDefaultLocaleNameProc = NULL;
    HRESULT hr = SkGetGetUserDefaultLocaleNameProc(&getUserDefaultLocaleNameProc);
    if (NULL == getUserDefaultLocaleNameProc) {
        SK_TRACEHR(hr, "Could not get GetUserDefaultLocaleName.");
    } else {
        localeNameLen = getUserDefaultLocaleNameProc(localeNameStorage, LOCALE_NAME_MAX_LENGTH);
        if (localeNameLen) {
            localeName = localeNameStorage;
        };
    }

    return SkNEW_ARGS(SkFontMgr_DirectWrite, (factory, sysFontCollection.get(),
                                              localeName, localeNameLen));
}

#include "SkFontMgr_indirect.h"
SK_API SkFontMgr* SkFontMgr_New_DirectWriteRenderer(SkRemotableFontMgr* proxy) {
    SkAutoTUnref<SkFontMgr> impl(SkFontMgr_New_DirectWrite());
    if (impl.get() == NULL) {
        return NULL;
    }
    return SkNEW_ARGS(SkFontMgr_Indirect, (impl.get(), proxy));
}
