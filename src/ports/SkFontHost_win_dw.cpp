/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#undef GetGlyphIndices

#include "SkAdvancedTypefaceMetrics.h"
#include "SkColorFilter.h"
#include "SkDWriteFontFileStream.h"
#include "SkDWriteGeometrySink.h"
#include "SkDescriptor.h"
#include "SkEndian.h"
#include "SkFontDescriptor.h"
#include "SkFontHost.h"
#include "SkGlyph.h"
#include "SkHRESULT.h"
#include "SkMaskGamma.h"
#include "SkOTTable_head.h"
#include "SkOTTable_hhea.h"
#include "SkOTTable_OS_2.h"
#include "SkOTTable_post.h"
#include "SkPath.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTScopedComPtr.h"
#include "SkThread.h"
#include "SkTypeface_win.h"
#include "SkTypefaceCache.h"
#include "SkUtils.h"

#include <dwrite.h>

SK_DECLARE_STATIC_MUTEX(gFTMutex);

static bool isLCD(const SkScalerContext::Rec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat ||
           SkMask::kLCD32_Format == rec.fMaskFormat;
}

///////////////////////////////////////////////////////////////////////////////

class DWriteOffscreen {
public:
    DWriteOffscreen() : fWidth(0), fHeight(0) {
    }

    void init(IDWriteFontFace* fontFace, const DWRITE_MATRIX& xform, FLOAT fontSize) {
        fFontFace = fontFace;
        fFontSize = fontSize;
        fXform = xform;
    }

    const void* draw(const SkGlyph&, bool isBW);

private:
    uint16_t fWidth;
    uint16_t fHeight;
    IDWriteFontFace* fFontFace;
    FLOAT fFontSize;
    DWRITE_MATRIX fXform;
    SkTDArray<uint8_t> fBits;
};

typedef HRESULT (WINAPI *DWriteCreateFactoryProc)(
    __in DWRITE_FACTORY_TYPE factoryType,
    __in REFIID iid,
    __out IUnknown **factory
);

static HRESULT get_dwrite_factory(IDWriteFactory** factory) {
    static IDWriteFactory* gDWriteFactory = NULL;

    if (gDWriteFactory != NULL) {
        *factory = gDWriteFactory;
        return S_OK;
    }

    DWriteCreateFactoryProc dWriteCreateFactoryProc =
        reinterpret_cast<DWriteCreateFactoryProc>(
            GetProcAddress(LoadLibraryW(L"dwrite.dll"), "DWriteCreateFactory")
        )
    ;

    if (!dWriteCreateFactoryProc) {
        return E_UNEXPECTED;
    }

    HRM(dWriteCreateFactoryProc(DWRITE_FACTORY_TYPE_SHARED,
                                __uuidof(IDWriteFactory),
                                reinterpret_cast<IUnknown**>(&gDWriteFactory)),
        "Could not create DirectWrite factory.");

    *factory = gDWriteFactory;
    return S_OK;
}

const void* DWriteOffscreen::draw(const SkGlyph& glyph, bool isBW) {
    IDWriteFactory* factory;
    HRNM(get_dwrite_factory(&factory), "Could not get factory.");

    if (fWidth < glyph.fWidth || fHeight < glyph.fHeight) {
        fWidth = SkMax32(fWidth, glyph.fWidth);
        fHeight = SkMax32(fHeight, glyph.fHeight);

        if (isBW) {
            fBits.setCount(fWidth * fHeight);
        } else {
            fBits.setCount(fWidth * fHeight * 3);
        }
    }

    // erase
    memset(fBits.begin(), 0, fBits.count());

    fXform.dx = SkFixedToFloat(glyph.getSubXFixed());
    fXform.dy = SkFixedToFloat(glyph.getSubYFixed());

    FLOAT advance = 0.0f;

    UINT16 index = glyph.getGlyphID();

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0.0f;
    offset.ascenderOffset = 0.0f;

    DWRITE_GLYPH_RUN run;
    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = fFontFace;
    run.fontEmSize = fFontSize;
    run.bidiLevel = 0;
    run.glyphIndices = &index;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    DWRITE_RENDERING_MODE renderingMode;
    DWRITE_TEXTURE_TYPE textureType;
    if (isBW) {
        renderingMode = DWRITE_RENDERING_MODE_ALIASED;
        textureType = DWRITE_TEXTURE_ALIASED_1x1;
    } else {
        renderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
        textureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
    }
    SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
    HRNM(factory->CreateGlyphRunAnalysis(&run,
                                         1.0f, // pixelsPerDip,
                                         &fXform,
                                         renderingMode,
                                         DWRITE_MEASURING_MODE_NATURAL,
                                         0.0f, // baselineOriginX,
                                         0.0f, // baselineOriginY,
                                         &glyphRunAnalysis),
         "Could not create glyph run analysis.");

    //NOTE: this assumes that the glyph has already been measured
    //with an exact same glyph run analysis.
    RECT bbox;
    bbox.left = glyph.fLeft;
    bbox.top = glyph.fTop;
    bbox.right = glyph.fLeft + glyph.fWidth;
    bbox.bottom = glyph.fTop + glyph.fHeight;
    HRNM(glyphRunAnalysis->CreateAlphaTexture(textureType,
                                              &bbox,
                                              fBits.begin(),
                                              fBits.count()),
         "Could not draw mask.");
    return fBits.begin();
}

///////////////////////////////////////////////////////////////////////////////

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
    StreamFontFileLoader(SkStream* stream) : fRefCount(1), fStream(stream) {
        stream->ref();
    }

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
    ULONG fRefCount;

    SkTScopedComPtr<IDWriteFactory> fFactory;
    SkTScopedComPtr<IDWriteFontFile> fCurrentFile;
    SkTScopedComPtr<IDWriteFontFileLoader> fFontFileLoader;
    bool fHasNext;
};

StreamFontFileEnumerator::StreamFontFileEnumerator(IDWriteFactory* factory,
                                                   IDWriteFontFileLoader* fontFileLoader)
    : fRefCount(1)
    , fFactory(factory)
    , fCurrentFile()
    , fFontFileLoader(fontFileLoader)
    , fHasNext(true)
{
    factory->AddRef();
    fontFileLoader->AddRef();
}

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

    fCurrentFile.get()->AddRef();
    *fontFile = fCurrentFile.get();
    return  S_OK;
}

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
        , fFontFileLoader(fontFileLoader)
    {
        fontFileLoader->AddRef();
    }

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

///////////////////////////////////////////////////////////////////////////////

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
                       IDWriteFontFace* fontFace,
                       IDWriteFont* font,
                       IDWriteFontFamily* fontFamily,
                       StreamFontFileLoader* fontFileLoader = NULL,
                       IDWriteFontCollectionLoader* fontCollectionLoader = NULL)
        : SkTypeface(style, fontID, false)
        , fDWriteFontCollectionLoader(fontCollectionLoader)
        , fDWriteFontFileLoader(fontFileLoader)
        , fDWriteFontFamily(fontFamily)
        , fDWriteFont(font)
        , fDWriteFontFace(fontFace) {

        if (fontCollectionLoader != NULL) {
            fontCollectionLoader->AddRef();
        }
        if (fontFileLoader != NULL) {
            fontFileLoader->AddRef();
        }
        fontFamily->AddRef();
        font->AddRef();
        fontFace->AddRef();
    }

public:
    SkTScopedComPtr<IDWriteFontCollectionLoader> fDWriteFontCollectionLoader;
    SkTScopedComPtr<StreamFontFileLoader> fDWriteFontFileLoader;
    SkTScopedComPtr<IDWriteFontFamily> fDWriteFontFamily;
    SkTScopedComPtr<IDWriteFont> fDWriteFont;
    SkTScopedComPtr<IDWriteFontFace> fDWriteFontFace;

    static DWriteFontTypeface* Create(IDWriteFontFace* fontFace,
                                      IDWriteFont* font,
                                      IDWriteFontFamily* fontFamily,
                                      StreamFontFileLoader* fontFileLoader = NULL,
                                      IDWriteFontCollectionLoader* fontCollectionLoader = NULL) {
        SkTypeface::Style style = get_style(font);
        SkFontID fontID = SkTypefaceCache::NewFontID();
        return SkNEW_ARGS(DWriteFontTypeface, (style, fontID,
                                               fontFace, font, fontFamily,
                                               fontFileLoader, fontCollectionLoader));
    }

    ~DWriteFontTypeface() {
        if (fDWriteFontCollectionLoader.get() == NULL) return;

        IDWriteFactory* factory;
        HRVM(get_dwrite_factory(&factory), "Could not get factory.");
        HRV(factory->UnregisterFontCollectionLoader(fDWriteFontCollectionLoader.get()));
        HRV(factory->UnregisterFontFileLoader(fDWriteFontFileLoader.get()));
    }

protected:
    virtual SkStream* onOpenStream(int* ttcIndex) const SK_OVERRIDE;
    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const SK_OVERRIDE;
    virtual void onFilterRec(SkScalerContextRec*) const SK_OVERRIDE;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                SkAdvancedTypefaceMetrics::PerGlyphInfo,
                                const uint32_t*, uint32_t) const SK_OVERRIDE;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool*) const SK_OVERRIDE;
};

class SkScalerContext_Windows : public SkScalerContext {
public:
    SkScalerContext_Windows(DWriteFontTypeface*, const SkDescriptor* desc);
    virtual ~SkScalerContext_Windows();

protected:
    virtual unsigned generateGlyphCount() SK_OVERRIDE;
    virtual uint16_t generateCharToGlyph(SkUnichar uni) SK_OVERRIDE;
    virtual void generateAdvance(SkGlyph* glyph) SK_OVERRIDE;
    virtual void generateMetrics(SkGlyph* glyph) SK_OVERRIDE;
    virtual void generateImage(const SkGlyph& glyph) SK_OVERRIDE;
    virtual void generatePath(const SkGlyph& glyph, SkPath* path) SK_OVERRIDE;
    virtual void generateFontMetrics(SkPaint::FontMetrics* mX,
                                     SkPaint::FontMetrics* mY) SK_OVERRIDE;

private:
    DWriteOffscreen fOffscreen;
    DWRITE_MATRIX fXform;
    SkAutoTUnref<DWriteFontTypeface> fTypeface;
    int fGlyphCount;
};

#define SK_DWRITE_DEFAULT_FONT_NAMED 1
#define SK_DWRITE_DEFAULT_FONT_MESSAGE 2
#define SK_DWRITE_DEFAULT_FONT_THEME 3
#define SK_DWRITE_DEFAULT_FONT_SHELLDLG 4
#define SK_DWRITE_DEFAULT_FONT_GDI 5
#define SK_DWRITE_DEFAULT_FONT_STRATEGY SK_DWRITE_DEFAULT_FONT_MESSAGE

static HRESULT get_default_font(IDWriteFont** font) {
    IDWriteFactory* factory;
    HRM(get_dwrite_factory(&factory), "Could not get factory.");

#if SK_DWRITE_DEFAULT_FONT_STRATEGY == SK_DWRITE_DEFAULT_FONT_NAMED
    SkTScopedComPtr<IDWriteFontCollection> sysFonts;
    HRM(factory->GetSystemFontCollection(&sysFonts, false),
        "Could not get system font collection.");

    UINT32 index;
    BOOL exists;
    //hr = sysFonts->FindFamilyName(L"Georgia", &index, &exists);
    HRM(sysFonts->FindFamilyName(L"Microsoft Sans Serif", &index, &exists),
        "Could not access family names.");

    if (!exists) {
        SkDEBUGF(("The hard coded font family does not exist."));
        return E_UNEXPECTED;
    }

    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRM(sysFonts->GetFontFamily(index, &fontFamily),
        "Could not load the requested font family.");

    HRM(fontFamily->GetFont(0, font), "Could not get first font from family.");

#elif SK_DWRITE_DEFAULT_FONT_STRATEGY == SK_DWRITE_DEFAULT_FONT_MESSAGE
    SkTScopedComPtr<IDWriteGdiInterop> gdi;
    HRM(factory->GetGdiInterop(&gdi), "Could not get GDI interop.");

    NONCLIENTMETRICSW metrics;
    metrics.cbSize = sizeof(metrics);
    if (0 == SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,
                                   sizeof(metrics),
                                   &metrics,
                                   0)) {
        return E_UNEXPECTED;
    }
    HRM(gdi->CreateFontFromLOGFONT(&metrics.lfMessageFont, font),
        "Could not create DWrite font from LOGFONT.");

#elif SK_DWRITE_DEFAULT_FONT_STRATEGY == SK_DWRITE_DEFAULT_FONT_THEME
    //Theme body font?

#elif SK_DWRITE_DEFAULT_FONT_STRATEGY == SK_DWRITE_DEFAULT_FONT_SHELLDLG
    //"MS Shell Dlg" or "MS Shell Dlg 2"?

#elif SK_DWRITE_DEFAULT_FONT_STRATEGY == SK_DWRITE_DEFAULT_FONT_GDI
    //Never works.
    SkTScopedComPtr<IDWriteGdiInterop> gdi;
    HRM(factory->GetGdiInterop(&gdi), "Could not get GDI interop.");

    static LOGFONTW gDefaultFont = {};
    gDefaultFont.lfFaceName
    HRM(gdi->CreateFontFromLOGFONT(&gDefaultFont, font),
        "Could not create DWrite font from LOGFONT.";
#endif
    return S_OK;
}

static bool are_same(IUnknown* a, IUnknown* b) {
    SkTScopedComPtr<IUnknown> iunkA;
    if (FAILED(a->QueryInterface(&iunkA))) {
        return false;
    }

    SkTScopedComPtr<IUnknown> iunkB;
    if (FAILED(b->QueryInterface(&iunkB))) {
        return false;
    }

    return iunkA.get() == iunkB.get();
}
static bool FindByDWriteFont(SkTypeface* face, SkTypeface::Style requestedStyle, void* ctx) {
    //Check to see if the two fonts are identical.
    DWriteFontTypeface* dwFace = reinterpret_cast<DWriteFontTypeface*>(face);
    IDWriteFont* dwFont = reinterpret_cast<IDWriteFont*>(ctx);
    if (are_same(dwFace->fDWriteFont.get(), dwFont)) {
        return true;
    }

    //Check if the two fonts share the same loader and have the same key.
    SkTScopedComPtr<IDWriteFontFace> dwFaceFontFace;
    SkTScopedComPtr<IDWriteFontFace> dwFontFace;
    HRB(dwFace->fDWriteFont->CreateFontFace(&dwFaceFontFace));
    HRB(dwFont->CreateFontFace(&dwFontFace));
    if (are_same(dwFaceFontFace.get(), dwFontFace.get())) {
        return true;
    }

    UINT32 dwFaceNumFiles;
    UINT32 dwNumFiles;
    HRB(dwFaceFontFace->GetFiles(&dwFaceNumFiles, NULL));
    HRB(dwFontFace->GetFiles(&dwNumFiles, NULL));
    if (dwFaceNumFiles != dwNumFiles) {
        return false;
    }

    SkTScopedComPtr<IDWriteFontFile> dwFaceFontFile;
    SkTScopedComPtr<IDWriteFontFile> dwFontFile;
    HRB(dwFaceFontFace->GetFiles(&dwFaceNumFiles, &dwFaceFontFile));
    HRB(dwFontFace->GetFiles(&dwNumFiles, &dwFontFile));

    //for (each file) { //we currently only admit fonts from one file.
    SkTScopedComPtr<IDWriteFontFileLoader> dwFaceFontFileLoader;
    SkTScopedComPtr<IDWriteFontFileLoader> dwFontFileLoader;
    HRB(dwFaceFontFile->GetLoader(&dwFaceFontFileLoader));
    HRB(dwFontFile->GetLoader(&dwFontFileLoader));
    if (!are_same(dwFaceFontFileLoader.get(), dwFontFileLoader.get())) {
        return false;
    }
    //}

    const void* dwFaceFontRefKey;
    UINT32 dwFaceFontRefKeySize;
    const void* dwFontRefKey;
    UINT32 dwFontRefKeySize;
    HRB(dwFaceFontFile->GetReferenceKey(&dwFaceFontRefKey, &dwFaceFontRefKeySize));
    HRB(dwFontFile->GetReferenceKey(&dwFontRefKey, &dwFontRefKeySize));
    if (dwFaceFontRefKeySize != dwFontRefKeySize) {
        return false;
    }
    if (0 != memcmp(dwFaceFontRefKey, dwFontRefKey, dwFontRefKeySize)) {
        return false;
    }

    //TODO: better means than comparing name strings?
    //NOTE: .tfc and fake bold/italic will end up here.
    SkTScopedComPtr<IDWriteFontFamily> dwFaceFontFamily;
    SkTScopedComPtr<IDWriteFontFamily> dwFontFamily;
    HRB(dwFace->fDWriteFont->GetFontFamily(&dwFaceFontFamily));
    HRB(dwFont->GetFontFamily(&dwFontFamily));

    SkTScopedComPtr<IDWriteLocalizedStrings> dwFaceFontFamilyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> dwFaceFontNames;
    HRB(dwFaceFontFamily->GetFamilyNames(&dwFaceFontFamilyNames));
    HRB(dwFace->fDWriteFont->GetFaceNames(&dwFaceFontNames));

    SkTScopedComPtr<IDWriteLocalizedStrings> dwFontFamilyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> dwFontNames;
    HRB(dwFontFamily->GetFamilyNames(&dwFontFamilyNames));
    HRB(dwFont->GetFaceNames(&dwFontNames));

    UINT32 dwFaceFontFamilyNameLength;
    UINT32 dwFaceFontNameLength;
    HRB(dwFaceFontFamilyNames->GetStringLength(0, &dwFaceFontFamilyNameLength));
    HRB(dwFaceFontNames->GetStringLength(0, &dwFaceFontNameLength));

    UINT32 dwFontFamilyNameLength;
    UINT32 dwFontNameLength;
    HRB(dwFontFamilyNames->GetStringLength(0, &dwFontFamilyNameLength));
    HRB(dwFontNames->GetStringLength(0, &dwFontNameLength));

    if (dwFaceFontFamilyNameLength != dwFontFamilyNameLength ||
        dwFaceFontNameLength != dwFontNameLength)
    {
        return false;
    }

    SkTDArray<wchar_t> dwFaceFontFamilyNameChar(new wchar_t[dwFaceFontFamilyNameLength+1], dwFaceFontFamilyNameLength+1);
    SkTDArray<wchar_t> dwFaceFontNameChar(new wchar_t[dwFaceFontNameLength+1], dwFaceFontNameLength+1);
    HRB(dwFaceFontFamilyNames->GetString(0, dwFaceFontFamilyNameChar.begin(), dwFaceFontFamilyNameChar.count()));
    HRB(dwFaceFontNames->GetString(0, dwFaceFontNameChar.begin(), dwFaceFontNameChar.count()));

    SkTDArray<wchar_t> dwFontFamilyNameChar(new wchar_t[dwFontFamilyNameLength+1], dwFontFamilyNameLength+1);
    SkTDArray<wchar_t> dwFontNameChar(new wchar_t[dwFontNameLength+1], dwFontNameLength+1);
    HRB(dwFontFamilyNames->GetString(0, dwFontFamilyNameChar.begin(), dwFontFamilyNameChar.count()));
    HRB(dwFontNames->GetString(0, dwFontNameChar.begin(), dwFontNameChar.count()));

    return wcscmp(dwFaceFontFamilyNameChar.begin(), dwFontFamilyNameChar.begin()) == 0 &&
           wcscmp(dwFaceFontNameChar.begin(), dwFontNameChar.begin()) == 0;
}

SkTypeface* SkCreateTypefaceFromDWriteFont(IDWriteFontFace* fontFace,
                                           IDWriteFont* font,
                                           IDWriteFontFamily* fontFamily,
                                           StreamFontFileLoader* fontFileLoader = NULL,
                                           IDWriteFontCollectionLoader* fontCollectionLoader = NULL) {
    SkTypeface* face = SkTypefaceCache::FindByProcAndRef(FindByDWriteFont, font);
    if (NULL == face) {
        face = DWriteFontTypeface::Create(fontFace, font, fontFamily,
                                          fontFileLoader, fontCollectionLoader);
        SkTypefaceCache::Add(face, get_style(font), fontCollectionLoader != NULL);
    }
    return face;
}

void SkDWriteFontFromTypeface(const SkTypeface* face, IDWriteFont** font) {
    if (NULL == face) {
        HRVM(get_default_font(font), "Could not get default font.");
    } else {
        *font = static_cast<const DWriteFontTypeface*>(face)->fDWriteFont.get();
        (*font)->AddRef();
    }
}
static DWriteFontTypeface* GetDWriteFontByID(SkFontID fontID) {
    return static_cast<DWriteFontTypeface*>(SkTypefaceCache::FindByID(fontID));
}

SkScalerContext_Windows::SkScalerContext_Windows(DWriteFontTypeface* typeface,
                                                 const SkDescriptor* desc)
        : SkScalerContext(typeface, desc)
        , fTypeface(SkRef(typeface))
        , fGlyphCount(-1) {
    SkAutoMutexAcquire ac(gFTMutex);

    fXform.m11 = SkScalarToFloat(fRec.fPost2x2[0][0]);
    fXform.m12 = SkScalarToFloat(fRec.fPost2x2[1][0]);
    fXform.m21 = SkScalarToFloat(fRec.fPost2x2[0][1]);
    fXform.m22 = SkScalarToFloat(fRec.fPost2x2[1][1]);
    fXform.dx = 0;
    fXform.dy = 0;

    fOffscreen.init(fTypeface->fDWriteFontFace.get(), fXform, SkScalarToFloat(fRec.fTextSize));
}

SkScalerContext_Windows::~SkScalerContext_Windows() {
}

unsigned SkScalerContext_Windows::generateGlyphCount() {
    if (fGlyphCount < 0) {
        fGlyphCount = fTypeface->fDWriteFontFace->GetGlyphCount();
    }
    return fGlyphCount;
}

uint16_t SkScalerContext_Windows::generateCharToGlyph(SkUnichar uni) {
    uint16_t index = 0;
    fTypeface->fDWriteFontFace->GetGlyphIndices(reinterpret_cast<UINT32*>(&uni), 1, &index);
    return index;
}

void SkScalerContext_Windows::generateAdvance(SkGlyph* glyph) {
    //Delta is the difference between the right/left side bearing metric
    //and where the right/left side bearing ends up after hinting.
    //DirectWrite does not provide this information.
    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;

    glyph->fAdvanceX = 0;
    glyph->fAdvanceY = 0;

    uint16_t glyphId = glyph->getGlyphID();
    DWRITE_GLYPH_METRICS gm;
    HRVM(fTypeface->fDWriteFontFace->GetDesignGlyphMetrics(&glyphId, 1, &gm),
         "Could not get design metrics.");

    DWRITE_FONT_METRICS dwfm;
    fTypeface->fDWriteFontFace->GetMetrics(&dwfm);

    SkScalar advanceX = SkScalarMulDiv(fRec.fTextSize,
                                       SkIntToScalar(gm.advanceWidth),
                                       SkIntToScalar(dwfm.designUnitsPerEm));

    if (!(fRec.fFlags & kSubpixelPositioning_Flag)) {
        advanceX = SkScalarRoundToScalar(advanceX);
    }

    SkVector vecs[1] = { { advanceX, 0 } };
    SkMatrix mat;
    fRec.getMatrixFrom2x2(&mat);
    mat.mapVectors(vecs, SK_ARRAY_COUNT(vecs));

    glyph->fAdvanceX = SkScalarToFixed(vecs[0].fX);
    glyph->fAdvanceY = SkScalarToFixed(vecs[0].fY);
}

void SkScalerContext_Windows::generateMetrics(SkGlyph* glyph) {
    glyph->fWidth = 0;

    this->generateAdvance(glyph);

    //Measure raster size.
    fXform.dx = SkFixedToFloat(glyph->getSubXFixed());
    fXform.dy = SkFixedToFloat(glyph->getSubYFixed());

    FLOAT advance = 0;

    UINT16 glyphId = glyph->getGlyphID();

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0.0f;
    offset.ascenderOffset = 0.0f;

    DWRITE_GLYPH_RUN run;
    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = fTypeface->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fRec.fTextSize);
    run.bidiLevel = 0;
    run.glyphIndices = &glyphId;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    IDWriteFactory* factory;
    HRVM(get_dwrite_factory(&factory), "Could not get factory.");

    const bool isBW = SkMask::kBW_Format == fRec.fMaskFormat;
    DWRITE_RENDERING_MODE renderingMode;
    DWRITE_TEXTURE_TYPE textureType;
    if (isBW) {
        renderingMode = DWRITE_RENDERING_MODE_ALIASED;
        textureType = DWRITE_TEXTURE_ALIASED_1x1;
    } else {
        renderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
        textureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
    }

    SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
    HRVM(factory->CreateGlyphRunAnalysis(&run,
                                         1.0f, // pixelsPerDip,
                                         &fXform,
                                         renderingMode,
                                         DWRITE_MEASURING_MODE_NATURAL,
                                         0.0f, // baselineOriginX,
                                         0.0f, // baselineOriginY,
                                         &glyphRunAnalysis),
         "Could not create glyph run analysis.");

    RECT bbox;
    HRVM(glyphRunAnalysis->GetAlphaTextureBounds(textureType, &bbox),
         "Could not get texture bounds.");

    glyph->fWidth = SkToU16(bbox.right - bbox.left);
    glyph->fHeight = SkToU16(bbox.bottom - bbox.top);
    glyph->fLeft = SkToS16(bbox.left);
    glyph->fTop = SkToS16(bbox.top);
}

void SkScalerContext_Windows::generateFontMetrics(SkPaint::FontMetrics* mx,
                                                  SkPaint::FontMetrics* my) {
    if (!(mx || my))
      return;

    DWRITE_FONT_METRICS dwfm;
    fTypeface->fDWriteFontFace->GetMetrics(&dwfm);

    if (mx) {
        mx->fTop = SkScalarMulDiv(-fRec.fTextSize,
                                  SkIntToScalar(dwfm.ascent),
                                  SkIntToScalar(dwfm.designUnitsPerEm));
        mx->fAscent = mx->fTop;
        mx->fDescent = SkScalarMulDiv(fRec.fTextSize,
                                      SkIntToScalar(dwfm.descent),
                                      SkIntToScalar(dwfm.designUnitsPerEm));
        mx->fBottom = mx->fDescent;
        //TODO, can be less than zero
        mx->fLeading = SkScalarMulDiv(fRec.fTextSize,
                                      SkIntToScalar(dwfm.lineGap),
                                      SkIntToScalar(dwfm.designUnitsPerEm));
    }

    if (my) {
        my->fTop = SkScalarMulDiv(-fRec.fTextSize,
                                  SkIntToScalar(dwfm.ascent),
                                  SkIntToScalar(dwfm.designUnitsPerEm));
        my->fAscent = my->fTop;
        my->fDescent = SkScalarMulDiv(fRec.fTextSize,
                                      SkIntToScalar(dwfm.descent),
                                      SkIntToScalar(dwfm.designUnitsPerEm));
        my->fBottom = my->fDescent;
        //TODO, can be less than zero
        my->fLeading = SkScalarMulDiv(fRec.fTextSize,
                                      SkIntToScalar(dwfm.lineGap),
                                      SkIntToScalar(dwfm.designUnitsPerEm));
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"

static void bilevel_to_bw(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph) {
    const int width = glyph.fWidth;
    const size_t dstRB = (width + 7) >> 3;
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(glyph.fImage);

    int byteCount = width >> 3;
    int bitCount = width & 7;

    for (int y = 0; y < glyph.fHeight; ++y) {
        if (byteCount > 0) {
            for (int i = 0; i < byteCount; ++i) {
                unsigned byte = 0;
                byte |= src[0] & (1 << 7);
                byte |= src[1] & (1 << 6);
                byte |= src[2] & (1 << 5);
                byte |= src[3] & (1 << 4);
                byte |= src[4] & (1 << 3);
                byte |= src[5] & (1 << 2);
                byte |= src[6] & (1 << 1);
                byte |= src[7] & (1 << 0);
                dst[i] = byte;
                src += 8;
            }
        }
        if (bitCount > 0) {
            unsigned byte = 0;
            unsigned mask = 0x80;
            for (int i = 0; i < bitCount; i++) {
                byte |= (src[i]) & mask;
                mask >>= 1;
            }
            dst[byteCount] = byte;
        }
        src += bitCount;
        dst += dstRB;
    }
}

template<bool APPLY_PREBLEND>
static void rgb_to_a8(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph, const uint8_t* table8) {
    const size_t dstRB = glyph.rowBytes();
    const U16CPU width = glyph.fWidth;
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(glyph.fImage);

    for (U16CPU y = 0; y < glyph.fHeight; y++) {
        for (U16CPU i = 0; i < width; i++) {
            U8CPU r = *(src++);
            U8CPU g = *(src++);
            U8CPU b = *(src++);
            dst[i] = sk_apply_lut_if<APPLY_PREBLEND>((r + g + b) / 3, table8);
        }
        dst = (uint8_t*)((char*)dst + dstRB);
    }
}

template<bool APPLY_PREBLEND>
static void rgb_to_lcd16(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph,
                         const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const size_t dstRB = glyph.rowBytes();
    const U16CPU width = glyph.fWidth;
    uint16_t* SK_RESTRICT dst = static_cast<uint16_t*>(glyph.fImage);

    for (U16CPU y = 0; y < glyph.fHeight; y++) {
        for (U16CPU i = 0; i < width; i++) {
            U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableR);
            U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableG);
            U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableB);
            dst[i] = SkPack888ToRGB16(r, g, b);
        }
        dst = (uint16_t*)((char*)dst + dstRB);
    }
}

template<bool APPLY_PREBLEND>
static void rgb_to_lcd32(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph,
                         const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const size_t dstRB = glyph.rowBytes();
    const U16CPU width = glyph.fWidth;
    SkPMColor* SK_RESTRICT dst = static_cast<SkPMColor*>(glyph.fImage);

    for (U16CPU y = 0; y < glyph.fHeight; y++) {
        for (U16CPU i = 0; i < width; i++) {
            U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableR);
            U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableG);
            U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableB);
            dst[i] = SkPackARGB32(0xFF, r, g, b);
        }
        dst = (SkPMColor*)((char*)dst + dstRB);
    }
}

void SkScalerContext_Windows::generateImage(const SkGlyph& glyph) {
    SkAutoMutexAcquire ac(gFTMutex);

    const bool isBW = SkMask::kBW_Format == fRec.fMaskFormat;
    const bool isAA = !isLCD(fRec);

    //Create the mask.
    const void* bits = fOffscreen.draw(glyph, isBW);
    if (!bits) {
        sk_bzero(glyph.fImage, glyph.computeImageSize());
        return;
    }

    //Copy the mask into the glyph.
    const uint8_t* src = (const uint8_t*)bits;
    if (isBW) {
        bilevel_to_bw(src, glyph);
    } else if (isAA) {
        if (fPreBlend.isApplicable()) {
            rgb_to_a8<true>(src, glyph, fPreBlend.fG);
        } else {
            rgb_to_a8<false>(src, glyph, fPreBlend.fG);
        }
    } else if (SkMask::kLCD16_Format == glyph.fMaskFormat) {
        if (fPreBlend.isApplicable()) {
            rgb_to_lcd16<true>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
        } else {
            rgb_to_lcd16<false>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
        }
    } else {
        SkASSERT(SkMask::kLCD32_Format == glyph.fMaskFormat);
        if (fPreBlend.isApplicable()) {
            rgb_to_lcd32<true>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
        } else {
            rgb_to_lcd32<false>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
        }
    }
}

void SkScalerContext_Windows::generatePath(const SkGlyph& glyph, SkPath* path) {
    SkAutoMutexAcquire ac(gFTMutex);

    SkASSERT(&glyph && path);

    path->reset();

    SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
    HRVM(SkDWriteGeometrySink::Create(path, &geometryToPath),
         "Could not create geometry to path converter.");
    uint16_t glyphId = glyph.getGlyphID();
    //TODO: convert to<->from DIUs? This would make a difference if hinting.
    //It may not be needed, it appears that DirectWrite only hints at em size.
    HRVM(fTypeface->fDWriteFontFace->GetGlyphRunOutline(SkScalarToFloat(fRec.fTextSize),
                                       &glyphId,
                                       NULL, //advances
                                       NULL, //offsets
                                       1, //num glyphs
                                       FALSE, //sideways
                                       FALSE, //rtl
                                       geometryToPath.get()),
         "Could not create glyph outline.");

    SkMatrix mat;
    fRec.getMatrixFrom2x2(&mat);
    path->transform(mat);
}

void DWriteFontTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    // Get the family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> dwFamilyNames;
    HRV(fDWriteFontFamily->GetFamilyNames(&dwFamilyNames));

    UINT32 dwFamilyNamesLength;
    HRV(dwFamilyNames->GetStringLength(0, &dwFamilyNamesLength));

    SkTDArray<wchar_t> dwFamilyNameChar(new wchar_t[dwFamilyNamesLength+1], dwFamilyNamesLength+1);
    HRV(dwFamilyNames->GetString(0, dwFamilyNameChar.begin(), dwFamilyNameChar.count()));

    // Convert the family name to utf8.
    // Get the buffer size needed first.
    int str_len = WideCharToMultiByte(CP_UTF8, 0, dwFamilyNameChar.begin(), -1,
                                      NULL, 0, NULL, NULL);
    // Allocate a buffer (str_len already has terminating null accounted for).
    SkTDArray<char> utf8FamilyName(new char[str_len], str_len);
    // Now actually convert the string.
    str_len = WideCharToMultiByte(CP_UTF8, 0, dwFamilyNameChar.begin(), -1,
                                  utf8FamilyName.begin(), str_len, NULL, NULL);

    desc->setFamilyName(utf8FamilyName.begin());
    *isLocalStream = SkToBool(fDWriteFontFileLoader.get());
}

SkTypeface* SkFontHost::CreateTypefaceFromStream(SkStream* stream) {
    IDWriteFactory* factory;
    HRN(get_dwrite_factory(&factory));

    SkTScopedComPtr<StreamFontFileLoader> fontFileLoader;
    HRN(StreamFontFileLoader::Create(stream, &fontFileLoader));
    HRN(factory->RegisterFontFileLoader(fontFileLoader.get()));

    SkTScopedComPtr<StreamFontCollectionLoader> streamFontCollectionLoader;
    HRN(StreamFontCollectionLoader::Create(fontFileLoader.get(), &streamFontCollectionLoader));
    HRN(factory->RegisterFontCollectionLoader(streamFontCollectionLoader.get()));

    SkTScopedComPtr<IDWriteFontCollection> streamFontCollection;
    HRN(factory->CreateCustomFontCollection(streamFontCollectionLoader.get(), NULL, 0,
                                            &streamFontCollection));

    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    HRN(streamFontCollection->GetFontFamily(0, &fontFamily));

    SkTScopedComPtr<IDWriteFont> font;
    HRN(fontFamily->GetFont(0, &font));

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    HRN(font->CreateFontFace(&fontFace));

    return SkCreateTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get(),
                                          fontFileLoader.get(), streamFontCollectionLoader.get());
}

SkStream* DWriteFontTypeface::onOpenStream(int* ttcIndex) const {
    *ttcIndex = 0;

    UINT32 numFiles;
    HRNM(fDWriteFontFace->GetFiles(&numFiles, NULL),
         "Could not get number of font files.");
    if (numFiles != 1) {
        return NULL;
    }

    SkTScopedComPtr<IDWriteFontFile> fontFile;
    HRNM(fDWriteFontFace->GetFiles(&numFiles, &fontFile), "Could not get font files.");

    const void* fontFileKey;
    UINT32 fontFileKeySize;
    HRNM(fontFile->GetReferenceKey(&fontFileKey, &fontFileKeySize),
         "Could not get font file reference key.");

    SkTScopedComPtr<IDWriteFontFileLoader> fontFileLoader;
    HRNM(fontFile->GetLoader(&fontFileLoader), "Could not get font file loader.");

    SkTScopedComPtr<IDWriteFontFileStream> fontFileStream;
    HRNM(fontFileLoader->CreateStreamFromKey(fontFileKey, fontFileKeySize,
                                             &fontFileStream),
         "Could not create font file stream.");

    return SkNEW_ARGS(SkDWriteFontFileStream, (fontFileStream.get()));
}

SkScalerContext* DWriteFontTypeface::onCreateScalerContext(const SkDescriptor* desc) const {
    return SkNEW_ARGS(SkScalerContext_Windows, (const_cast<DWriteFontTypeface*>(this), desc));
}

static HRESULT get_by_family_name(const char familyName[], IDWriteFontFamily** fontFamily) {
    IDWriteFactory* factory;
    HR(get_dwrite_factory(&factory));

    SkTScopedComPtr<IDWriteFontCollection> sysFontCollection;
    HR(factory->GetSystemFontCollection(&sysFontCollection, FALSE));

    // Get the buffer size needed first.
    int wlen = ::MultiByteToWideChar(CP_UTF8, 0, familyName,-1, NULL, 0);
    if (0 == wlen) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    // Allocate a buffer
    SkTDArray<wchar_t> wideFamilyName(new wchar_t[wlen], wlen);
    // Now actually convert the string.
    wlen = ::MultiByteToWideChar(CP_UTF8, 0, familyName, -1,
                                    wideFamilyName.begin(), wlen);
    if (0 == wlen) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    UINT32 index;
    BOOL exists;
    HR(sysFontCollection->FindFamilyName(wideFamilyName.begin(), &index, &exists));

    if (exists) {
        HR(sysFontCollection->GetFontFamily(index, fontFamily));
        return S_OK;
    }
    return S_FALSE;
}

/** Return the closest matching typeface given either an existing family
 (specified by a typeface in that family) or by a familyName, and a
 requested style.
 1) If familyFace is null, use familyName.
 2) If familyName is null, use familyFace.
 3) If both are null, return the default font that best matches style
 This MUST not return NULL.
 */
SkTypeface* SkFontHost::CreateTypeface(const SkTypeface* familyFace,
                                       const char familyName[],
                                       SkTypeface::Style style) {
    HRESULT hr;
    SkTScopedComPtr<IDWriteFontFamily> fontFamily;
    SkTScopedComPtr<IDWriteFontCollectionLoader> fontCollectionLoader;
    SkTScopedComPtr<IDWriteFontFileLoader> fontFileLoader;
    if (familyFace) {
        const DWriteFontTypeface* face = static_cast<const DWriteFontTypeface*>(familyFace);
        face->fDWriteFontFamily.get()->AddRef();
        *(&fontFamily) = face->fDWriteFontFamily.get();

        if (face->fDWriteFontCollectionLoader.get() != NULL) {
            face->fDWriteFontCollectionLoader.get()->AddRef();
            *(&fontCollectionLoader) = face->fDWriteFontCollectionLoader.get();

            face->fDWriteFontFileLoader.get()->AddRef();
            *(&fontFileLoader) = face->fDWriteFontFileLoader.get();
        }

    } else if (familyName) {
        hr = get_by_family_name(familyName, &fontFamily);
    }

    if (NULL == fontFamily.get()) {
        //No good family found, go with default.
        SkTScopedComPtr<IDWriteFont> font;
        hr = get_default_font(&font);
        hr = font->GetFontFamily(&fontFamily);
    }

    SkTScopedComPtr<IDWriteFont> font;
    DWRITE_FONT_WEIGHT weight = (style & SkTypeface::kBold)
                                 ? DWRITE_FONT_WEIGHT_BOLD
                                 : DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_UNDEFINED;
    DWRITE_FONT_STYLE italic = (style & SkTypeface::kItalic)
                                ? DWRITE_FONT_STYLE_ITALIC
                                : DWRITE_FONT_STYLE_NORMAL;
    hr = fontFamily->GetFirstMatchingFont(weight, stretch, italic, &font);

    SkTScopedComPtr<IDWriteFontFace> fontFace;
    hr = font->CreateFontFace(&fontFace);

    return SkCreateTypefaceFromDWriteFont(fontFace.get(), font.get(), fontFamily.get());
}

SkTypeface* SkFontHost::CreateTypefaceFromFile(const char path[]) {
    printf("SkFontHost::CreateTypefaceFromFile unimplemented");
    return NULL;
}

void DWriteFontTypeface::onFilterRec(SkScalerContext::Rec* rec) const {
    unsigned flagsWeDontSupport = SkScalerContext::kDevKernText_Flag |
                                  SkScalerContext::kAutohinting_Flag |
                                  SkScalerContext::kEmbeddedBitmapText_Flag |
                                  SkScalerContext::kEmbolden_Flag |
                                  SkScalerContext::kLCD_BGROrder_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;
    rec->fFlags &= ~flagsWeDontSupport;

    SkPaint::Hinting h = rec->getHinting();
    // DirectWrite does not provide for hinting hints.
    h = SkPaint::kSlight_Hinting;
    rec->setHinting(h);

#if SK_FONT_HOST_USE_SYSTEM_SETTINGS
    IDWriteFactory* factory;
    if (SUCCEEDED(get_dwrite_factory(&factory))) {
        SkTScopedComPtr<IDWriteRenderingParams> defaultRenderingParams;
        if (SUCCEEDED(factory->CreateRenderingParams(&defaultRenderingParams))) {
            float gamma = defaultRenderingParams->GetGamma();
            rec->setDeviceGamma(SkFloatToScalar(gamma));
            rec->setPaintGamma(SkFloatToScalar(gamma));

            rec->setContrast(SkFloatToScalar(defaultRenderingParams->GetEnhancedContrast()));
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
//PDF Support

using namespace skia_advanced_typeface_metrics_utils;

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
// TODO(arthurhsu): Add support for conjugate pairs. It looks like that may
// require parsing the TTF cmap table (platform 4, encoding 12) directly instead
// of calling GetFontUnicodeRange().
// TODO(bungeman): This never does what anyone wants.
// What is really wanted is the text to glyphs mapping
static void populate_glyph_to_unicode(IDWriteFontFace* fontFace,
                                      const unsigned glyphCount,
                                      SkTDArray<SkUnichar>* glyphToUnicode) {
    HRESULT hr = S_OK;

    //Do this like free type instead
    UINT32 count = 0;
    for (UINT32 c = 0; c < 0x10FFFF; ++c) {
        UINT16 glyph;
        hr = fontFace->GetGlyphIndices(&c, 1, &glyph);
        if (glyph > 0) {
            ++count;
        }
    }

    SkAutoTArray<UINT32> chars(count);
    count = 0;
    for (UINT32 c = 0; c < 0x10FFFF; ++c) {
        UINT16 glyph;
        hr = fontFace->GetGlyphIndices(&c, 1, &glyph);
        if (glyph > 0) {
            chars[count] = c;
            ++count;
        }
    }

    SkAutoTArray<UINT16> glyph(count);
    fontFace->GetGlyphIndices(chars.get(), count, glyph.get());

    USHORT maxGlyph = 0;
    for (USHORT j = 0; j < count; ++j) {
        if (glyph[j] > maxGlyph) maxGlyph = glyph[j];
    }

    glyphToUnicode->setCount(maxGlyph+1);
    for (size_t j = 0; j < maxGlyph+1u; ++j) {
        (*glyphToUnicode)[j] = 0;
    }

    //'invert'
    for (USHORT j = 0; j < count; ++j) {
        if (glyph[j] < glyphCount && (*glyphToUnicode)[glyph[j]] == 0) {
            (*glyphToUnicode)[glyph[j]] = chars[j];
        }
    }
}

static bool getWidthAdvance(IDWriteFontFace* fontFace, int gId, int16_t* advance) {
    SkASSERT(advance);

    UINT16 glyphId = gId;
    DWRITE_GLYPH_METRICS gm;
    HRESULT hr = fontFace->GetDesignGlyphMetrics(&glyphId, 1, &gm);

    if (FAILED(hr)) {
        *advance = 0;
        return false;
    }

    *advance = gm.advanceWidth;
    return true;
}

template<typename T>
class AutoDWriteTable {
public:
    AutoDWriteTable(IDWriteFontFace* fontFace)
        : fFontFace(fontFace)
        , fExists(FALSE) {

        //fontFace->AddRef();
        const UINT32 tag = DWRITE_MAKE_OPENTYPE_TAG(T::TAG0,
                                                    T::TAG1,
                                                    T::TAG2,
                                                    T::TAG3);
        // TODO: need to check for failure
        /*HRESULT hr =*/ fontFace->TryGetFontTable(tag,
            reinterpret_cast<const void **>(&fData), &fSize, &fLock, &fExists);
    }
    ~AutoDWriteTable() {
        if (fExists) {
            fFontFace->ReleaseFontTable(fLock);
        }
    }
    const T* operator->() const { return fData; }

    const T* fData;
    UINT32 fSize;
    BOOL fExists;
private:
    //SkTScopedComPtr<IDWriteFontFace> fFontFace;
    IDWriteFontFace* fFontFace;
    void* fLock;
};

SkAdvancedTypefaceMetrics* DWriteFontTypeface::onGetAdvancedTypefaceMetrics(
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const {

    SkAdvancedTypefaceMetrics* info = NULL;

    HRESULT hr = S_OK;

    const unsigned glyphCount = fDWriteFontFace->GetGlyphCount();

    DWRITE_FONT_METRICS dwfm;
    fDWriteFontFace->GetMetrics(&dwfm);

    info = new SkAdvancedTypefaceMetrics;
    info->fEmSize = dwfm.designUnitsPerEm;
    info->fMultiMaster = false;
    info->fLastGlyphID = SkToU16(glyphCount - 1);
    info->fStyle = 0;


    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    SkTScopedComPtr<IDWriteLocalizedStrings> faceNames;
    hr = fDWriteFontFamily->GetFamilyNames(&familyNames);
    hr = fDWriteFont->GetFaceNames(&faceNames);

    UINT32 familyNameLength;
    hr = familyNames->GetStringLength(0, &familyNameLength);

    UINT32 faceNameLength;
    hr = faceNames->GetStringLength(0, &faceNameLength);

    size_t size = familyNameLength+1+faceNameLength+1;
    SkTDArray<wchar_t> wFamilyName(new wchar_t[size], size);
    hr = familyNames->GetString(0, wFamilyName.begin(), size);
    wFamilyName[familyNameLength] = L' ';
    hr = faceNames->GetString(0, &wFamilyName[familyNameLength+1], size - faceNameLength + 1);

    size_t str_len = WideCharToMultiByte(CP_UTF8, 0, wFamilyName.begin(), -1, NULL, 0, NULL, NULL);
    if (0 == str_len) {
        //TODO: error
    }
    SkTDArray<char> familyName(new char[str_len], str_len);
    str_len = WideCharToMultiByte(CP_UTF8, 0, wFamilyName.begin(), -1, familyName.begin(), str_len, NULL, NULL);
    if (0 == str_len) {
        //TODO: error
    }
    info->fFontName.set(familyName.begin(), str_len);

    if (perGlyphInfo & SkAdvancedTypefaceMetrics::kToUnicode_PerGlyphInfo) {
        populate_glyph_to_unicode(fDWriteFontFace.get(), glyphCount, &(info->fGlyphToUnicode));
    }

    DWRITE_FONT_FACE_TYPE fontType = fDWriteFontFace->GetType();
    if (fontType == DWRITE_FONT_FACE_TYPE_TRUETYPE ||
        fontType == DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION) {
        info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
    } else {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        info->fItalicAngle = 0;
        info->fAscent = dwfm.ascent;;
        info->fDescent = dwfm.descent;
        info->fStemV = 0;
        info->fCapHeight = dwfm.capHeight;
        info->fBBox = SkIRect::MakeEmpty();
        return info;
    }

    AutoDWriteTable<SkOTTableHead> headTable(fDWriteFontFace.get());
    AutoDWriteTable<SkOTTablePostScript> postTable(fDWriteFontFace.get());
    AutoDWriteTable<SkOTTableHorizontalHeader> hheaTable(fDWriteFontFace.get());
    AutoDWriteTable<SkOTTableOS2> os2Table(fDWriteFontFace.get());
    if (!headTable.fExists || !postTable.fExists || !hheaTable.fExists || !os2Table.fExists) {
        info->fItalicAngle = 0;
        info->fAscent = dwfm.ascent;;
        info->fDescent = dwfm.descent;
        info->fStemV = 0;
        info->fCapHeight = dwfm.capHeight;
        info->fBBox = SkIRect::MakeEmpty();
        return info;
    }

    //There exist CJK fonts which set the IsFixedPitch and Monospace bits,
    //but have full width, latin half-width, and half-width kana.
    bool fixedWidth = (postTable->isFixedPitch &&
                      (1 == SkEndian_SwapBE16(hheaTable->numberOfHMetrics)));
    //Monospace
    if (fixedWidth) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    //Italic
    if (os2Table->version.v0.fsSelection.field.Italic) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }
    //Script
    if (SkPanose::FamilyType::Script == os2Table->version.v0.panose.bFamilyType.value) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    //Serif
    } else if (SkPanose::FamilyType::TextAndDisplay == os2Table->version.v0.panose.bFamilyType.value &&
               SkPanose::Data::TextAndDisplay::SerifStyle::Triangle <= os2Table->version.v0.panose.data.textAndDisplay.bSerifStyle.value &&
               SkPanose::Data::TextAndDisplay::SerifStyle::NoFit != os2Table->version.v0.panose.data.textAndDisplay.bSerifStyle.value) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    }

    info->fItalicAngle = SkEndian_SwapBE32(postTable->italicAngle) >> 16;

    info->fAscent = SkToS16(dwfm.ascent);
    info->fDescent = SkToS16(dwfm.descent);
    info->fCapHeight = SkToS16(dwfm.capHeight);

    info->fBBox = SkIRect::MakeLTRB((int32_t)SkEndian_SwapBE16((uint16_t)headTable->xMin),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->yMax),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->xMax),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->yMin));

    //TODO: is this even desired? It seems PDF only wants this value for Type1
    //fonts, and we only get here for TrueType fonts.
    info->fStemV = 0;
    /*
    // Figure out a good guess for StemV - Min width of i, I, !, 1.
    // This probably isn't very good with an italic font.
    int16_t min_width = SHRT_MAX;
    info->fStemV = 0;
    char stem_chars[] = {'i', 'I', '!', '1'};
    for (size_t i = 0; i < SK_ARRAY_COUNT(stem_chars); i++) {
        ABC abcWidths;
        if (GetCharABCWidths(hdc, stem_chars[i], stem_chars[i], &abcWidths)) {
            int16_t width = abcWidths.abcB;
            if (width > 0 && width < min_width) {
                min_width = width;
                info->fStemV = min_width;
            }
        }
    }
    */

    // If Restricted, the font may not be embedded in a document.
    // If not Restricted, the font can be embedded.
    // If PreviewPrint, the embedding is read-only.
    if (os2Table->version.v0.fsType.field.Restricted) {
        info->fType = SkAdvancedTypefaceMetrics::kNotEmbeddable_Font;
    } else if (perGlyphInfo & SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo) {
        if (fixedWidth) {
            appendRange(&info->fGlyphWidths, 0);
            int16_t advance;
            getWidthAdvance(fDWriteFontFace.get(), 1, &advance);
            info->fGlyphWidths->fAdvance.append(1, &advance);
            finishRange(info->fGlyphWidths.get(), 0,
                        SkAdvancedTypefaceMetrics::WidthRange::kDefault);
        } else {
            info->fGlyphWidths.reset(
                getAdvanceData(fDWriteFontFace.get(),
                               glyphCount,
                               glyphIDs,
                               glyphIDsCount,
                               getWidthAdvance));
        }
    }

    return info;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkFontMgr.h"

SkFontMgr* SkFontMgr::Factory() {
    // todo
    return NULL;
}
