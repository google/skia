/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/win/SkDWriteNTDDI_VERSION.h"

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "src/base/SkLeanWindows.h"

// SkLeanWindows will include Windows.h, which will pull in all of the GDI defines.
// GDI #defines GetGlyphIndices to GetGlyphIndicesA or GetGlyphIndicesW, but
// IDWriteFontFace has a method called GetGlyphIndices. Since this file does
// not use GDI, undefing GetGlyphIndices makes things less confusing.
#undef GetGlyphIndices

#include "include/core/SkData.h"
#include "include/core/SkFontTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontStream.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/SkScalerContext_win_dw.h"
#include "src/ports/SkTypeface_win_dw.h"
#include "src/sfnt/SkOTTable_OS_2.h"
#include "src/sfnt/SkOTTable_fvar.h"
#include "src/sfnt/SkOTTable_head.h"
#include "src/sfnt/SkOTTable_hhea.h"
#include "src/sfnt/SkOTTable_post.h"
#include "src/sfnt/SkOTUtils.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkDWriteFontFileStream.h"

using namespace skia_private;

HRESULT DWriteFontTypeface::initializePalette() {
    if (!fIsColorFont) {
        return S_OK;
    }

    UINT32 dwPaletteCount = fDWriteFontFace2->GetColorPaletteCount();
    if (dwPaletteCount == 0) {
        return S_OK;
    }

    // Treat out of range palette index values as 0. Still apply overrides.
    // https://www.w3.org/TR/css-fonts-4/#base-palette-desc
    UINT32 basePaletteIndex = 0;
    if (SkTFitsIn<UINT32>(fRequestedPalette.index) &&
        SkTo<UINT32>(fRequestedPalette.index) < dwPaletteCount)
    {
        basePaletteIndex = fRequestedPalette.index;
    }

    UINT32 dwPaletteEntryCount = fDWriteFontFace2->GetPaletteEntryCount();
    AutoSTMalloc<8, DWRITE_COLOR_F> dwPaletteEntry(dwPaletteEntryCount);
    HRM(fDWriteFontFace2->GetPaletteEntries(basePaletteIndex,
                                            0, dwPaletteEntryCount,
                                            dwPaletteEntry),
        "Could not retrieve palette entries.");

    fPalette.reset(new SkColor[dwPaletteEntryCount]);
    fDWPalette.reset(new DWRITE_COLOR_F[dwPaletteEntryCount]);
    for (UINT32 i = 0; i < dwPaletteEntryCount; ++i) {
        fPalette[i] = SkColorSetARGB(sk_float_round2int(dwPaletteEntry[i].a * 255),
                                     sk_float_round2int(dwPaletteEntry[i].r * 255),
                                     sk_float_round2int(dwPaletteEntry[i].g * 255),
                                     sk_float_round2int(dwPaletteEntry[i].b * 255));
        fDWPalette[i] = dwPaletteEntry[i];
    }

    for (int i = 0; i < fRequestedPalette.overrideCount; ++i) {
        const SkFontArguments::Palette::Override& paletteOverride = fRequestedPalette.overrides[i];
        if (SkTFitsIn<UINT32>(paletteOverride.index) &&
            SkTo<UINT32>(paletteOverride.index) < dwPaletteEntryCount)
        {
            fPalette[paletteOverride.index] = paletteOverride.color;

            // Avoid brace initialization as DWRITE_COLOR_F can be defined as four floats
            // (dxgitype.h, d3d9types.h) or four unions of two floats (dwrite_2.h, d3dtypes.h).
            // The type changed in Direct3D 10, but the change does not appear to be documented.
            const SkColor4f skColor = SkColor4f::FromColor(paletteOverride.color);
            DWRITE_COLOR_F& dwColor = fDWPalette[paletteOverride.index];
            dwColor.r = skColor.fR;
            dwColor.g = skColor.fG;
            dwColor.b = skColor.fB;
            dwColor.a = skColor.fA;
        }
    }
    fPaletteEntryCount = dwPaletteEntryCount;

    return S_OK;
}

DWriteFontTypeface::DWriteFontTypeface(const SkFontStyle& style,
                                       IDWriteFactory* factory,
                                       IDWriteFontFace* fontFace,
                                       IDWriteFont* font,
                                       IDWriteFontFamily* fontFamily,
                                       sk_sp<Loaders> loaders,
                                       const SkFontArguments::Palette& palette)
    : SkTypeface(style, false)
    , fFactory(SkRefComPtr(factory))
    , fDWriteFontFamily(SkRefComPtr(fontFamily))
    , fDWriteFont(SkRefComPtr(font))
    , fDWriteFontFace(SkRefComPtr(fontFace))
    , fRequestedPaletteEntryOverrides(palette.overrideCount
        ? (SkFontArguments::Palette::Override*)memcpy(
             new SkFontArguments::Palette::Override[palette.overrideCount],
             palette.overrides,
             palette.overrideCount * sizeof(palette.overrides[0]))
        : nullptr)
    , fRequestedPalette{palette.index,
                        fRequestedPaletteEntryOverrides.get(), palette.overrideCount }
    , fPaletteEntryCount(0)
    , fLoaders(std::move(loaders))
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
#if DWRITE_CORE || (defined(NTDDI_WIN11_ZN) && NTDDI_VERSION >= NTDDI_WIN11_ZN)
    if (!SUCCEEDED(fDWriteFontFace->QueryInterface(&fDWriteFontFace7))) {
        SkASSERT_RELEASE(nullptr == fDWriteFontFace7/*.get()*/);
    }
#endif
    if (!SUCCEEDED(fFactory->QueryInterface(&fFactory2))) {
        SkASSERT_RELEASE(nullptr == fFactory2.get());
    }

    if (fDWriteFontFace1 && fDWriteFontFace1->IsMonospacedFont()) {
        this->setIsFixedPitch(true);
    }

    fIsColorFont = fFactory2 && fDWriteFontFace2 && fDWriteFontFace2->IsColorFont();
    this->initializePalette();
}

DWriteFontTypeface::~DWriteFontTypeface() {
#if DWRITE_CORE || (defined(NTDDI_WIN11_ZN) && NTDDI_VERSION >= NTDDI_WIN11_ZN)
    if (fDWriteFontFace7) {
        fDWriteFontFace7->Release();
    }
#endif
}

DWriteFontTypeface::Loaders::~Loaders() {
    // Don't return if any fail, just keep going to free up as much as possible.
    HRESULT hr;

    hr = fFactory->UnregisterFontCollectionLoader(fDWriteFontCollectionLoader.get());
    if (FAILED(hr)) {
        SK_TRACEHR(hr, "FontCollectionLoader");
    }

    hr = fFactory->UnregisterFontFileLoader(fDWriteFontFileLoader.get());
    if (FAILED(hr)) {
        SK_TRACEHR(hr, "FontFileLoader");
    }
}

void DWriteFontTypeface::onGetFamilyName(SkString* familyName) const {
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRV(fDWriteFontFamily->GetFamilyNames(&familyNames));

    sk_get_locale_string(familyNames.get(), nullptr/*fMgr->fLocaleName.get()*/, familyName);
}

bool DWriteFontTypeface::onGetPostScriptName(SkString* skPostScriptName) const {
    SkString localSkPostScriptName;
    SkTScopedComPtr<IDWriteLocalizedStrings> postScriptNames;
    BOOL exists = FALSE;
    if (FAILED(fDWriteFont->GetInformationalStrings(
                    DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME,
                    &postScriptNames,
                    &exists)) ||
        !exists ||
        FAILED(sk_get_locale_string(postScriptNames.get(), nullptr, &localSkPostScriptName)))
    {
        return false;
    }
    if (skPostScriptName) {
        *skPostScriptName = localSkPostScriptName;
    }
    return true;
}

void DWriteFontTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* serialize) const {
    // Get the family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRV(fDWriteFontFamily->GetFamilyNames(&familyNames));

    SkString utf8FamilyName;
    sk_get_locale_string(familyNames.get(), nullptr/*fMgr->fLocaleName.get()*/, &utf8FamilyName);

    desc->setFamilyName(utf8FamilyName.c_str());
    desc->setStyle(this->fontStyle());

    desc->setPaletteIndex(fRequestedPalette.index);
    sk_careful_memcpy(desc->setPaletteEntryOverrides(fRequestedPalette.overrideCount),
                      fRequestedPalette.overrides,
                      fRequestedPalette.overrideCount * sizeof(fRequestedPalette.overrides[0]));

    desc->setFactoryId(FactoryId);
    *serialize = SkToBool(fLoaders);
}

void DWriteFontTypeface::onCharsToGlyphs(const SkUnichar* uni, int count,
                                         SkGlyphID glyphs[]) const {
    fDWriteFontFace->GetGlyphIndices((const UINT32*)uni, count, glyphs);
}

int DWriteFontTypeface::onCountGlyphs() const {
    return fDWriteFontFace->GetGlyphCount();
}

void DWriteFontTypeface::getPostScriptGlyphNames(SkString*) const {}

int DWriteFontTypeface::onGetUPEM() const {
    DWRITE_FONT_METRICS metrics;
    fDWriteFontFace->GetMetrics(&metrics);
    return metrics.designUnitsPerEm;
}

class LocalizedStrings_IDWriteLocalizedStrings : public SkTypeface::LocalizedStrings {
public:
    /** Takes ownership of the IDWriteLocalizedStrings. */
    explicit LocalizedStrings_IDWriteLocalizedStrings(IDWriteLocalizedStrings* strings)
        : fIndex(0), fStrings(strings)
    { }

    bool next(SkTypeface::LocalizedString* localizedString) override {
        if (fIndex >= fStrings->GetCount()) {
            return false;
        }

        // String
        UINT32 stringLen;
        HRBM(fStrings->GetStringLength(fIndex, &stringLen), "Could not get string length.");

        SkSMallocWCHAR wString(static_cast<size_t>(stringLen)+1);
        HRBM(fStrings->GetString(fIndex, wString.get(), stringLen+1), "Could not get string.");

        HRB(sk_wchar_to_skstring(wString.get(), stringLen, &localizedString->fString));

        // Locale
        UINT32 localeLen;
        HRBM(fStrings->GetLocaleNameLength(fIndex, &localeLen), "Could not get locale length.");

        SkSMallocWCHAR wLocale(static_cast<size_t>(localeLen)+1);
        HRBM(fStrings->GetLocaleName(fIndex, wLocale.get(), localeLen+1), "Could not get locale.");

        HRB(sk_wchar_to_skstring(wLocale.get(), localeLen, &localizedString->fLanguage));

        ++fIndex;
        return true;
    }

private:
    UINT32 fIndex;
    SkTScopedComPtr<IDWriteLocalizedStrings> fStrings;
};

SkTypeface::LocalizedStrings* DWriteFontTypeface::onCreateFamilyNameIterator() const {
    sk_sp<SkTypeface::LocalizedStrings> nameIter =
        SkOTUtils::LocalizedStrings_NameTable::MakeForFamilyNames(*this);
    if (!nameIter) {
        SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
        HRNM(fDWriteFontFamily->GetFamilyNames(&familyNames), "Could not obtain family names.");
        nameIter = sk_make_sp<LocalizedStrings_IDWriteLocalizedStrings>(familyNames.release());
    }
    return nameIter.release();
}

bool DWriteFontTypeface::onGlyphMaskNeedsCurrentColor() const {
    return fDWriteFontFace2 && fDWriteFontFace2->GetColorPaletteCount() > 0;
}

int DWriteFontTypeface::onGetVariationDesignPosition(
    SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const
{

#if defined(NTDDI_WIN10_RS3) && NTDDI_VERSION >= NTDDI_WIN10_RS3

    SkTScopedComPtr<IDWriteFontFace5> fontFace5;
    if (FAILED(fDWriteFontFace->QueryInterface(&fontFace5))) {
        return -1;
    }

    // Return 0 if the font is not variable font.
    if (!fontFace5->HasVariations()) {
        return 0;
    }

    UINT32 fontAxisCount = fontFace5->GetFontAxisValueCount();
    SkTScopedComPtr<IDWriteFontResource> fontResource;
    HR_GENERAL(fontFace5->GetFontResource(&fontResource), nullptr, -1);
    UINT32 variableAxisCount = 0;
    for (UINT32 i = 0; i < fontAxisCount; ++i) {
        if (fontResource->GetFontAxisAttributes(i) & DWRITE_FONT_AXIS_ATTRIBUTES_VARIABLE) {
            ++variableAxisCount;
        }
    }

    if (!coordinates || coordinateCount < 0 || (unsigned)coordinateCount < variableAxisCount) {
        return SkTo<int>(variableAxisCount);
    }

    AutoSTMalloc<8, DWRITE_FONT_AXIS_VALUE> fontAxisValue(fontAxisCount);
    HR_GENERAL(fontFace5->GetFontAxisValues(fontAxisValue.get(), fontAxisCount), nullptr, -1);
    UINT32 coordIndex = 0;
    for (UINT32 axisIndex = 0; axisIndex < fontAxisCount; ++axisIndex) {
        if (fontResource->GetFontAxisAttributes(axisIndex) & DWRITE_FONT_AXIS_ATTRIBUTES_VARIABLE) {
            coordinates[coordIndex].axis = SkEndian_SwapBE32(fontAxisValue[axisIndex].axisTag);
            coordinates[coordIndex].value = fontAxisValue[axisIndex].value;
            ++coordIndex;
        }
    }

    SkASSERT(coordIndex == variableAxisCount);
    return SkTo<int>(variableAxisCount);

#else
    return -1;
#endif
}

int DWriteFontTypeface::onGetVariationDesignParameters(
    SkFontParameters::Variation::Axis parameters[], int parameterCount) const
{

#if defined(NTDDI_WIN10_RS3) && NTDDI_VERSION >= NTDDI_WIN10_RS3

    SkTScopedComPtr<IDWriteFontFace5> fontFace5;
    if (FAILED(fDWriteFontFace->QueryInterface(&fontFace5))) {
        return -1;
    }

    // Return 0 if the font is not variable font.
    if (!fontFace5->HasVariations()) {
        return 0;
    }

    UINT32 fontAxisCount = fontFace5->GetFontAxisValueCount();
    SkTScopedComPtr<IDWriteFontResource> fontResource;
    HR_GENERAL(fontFace5->GetFontResource(&fontResource), nullptr, -1);
    int variableAxisCount = 0;
    for (UINT32 i = 0; i < fontAxisCount; ++i) {
        if (fontResource->GetFontAxisAttributes(i) & DWRITE_FONT_AXIS_ATTRIBUTES_VARIABLE) {
            variableAxisCount++;
        }
    }

    if (!parameters || parameterCount < variableAxisCount) {
        return variableAxisCount;
    }

    AutoSTMalloc<8, DWRITE_FONT_AXIS_RANGE> fontAxisRange(fontAxisCount);
    HR_GENERAL(fontResource->GetFontAxisRanges(fontAxisRange.get(), fontAxisCount), nullptr, -1);
    AutoSTMalloc<8, DWRITE_FONT_AXIS_VALUE> fontAxisDefaultValue(fontAxisCount);
    HR_GENERAL(fontResource->GetDefaultFontAxisValues(fontAxisDefaultValue.get(), fontAxisCount),
               nullptr, -1);
    UINT32 coordIndex = 0;

    for (UINT32 axisIndex = 0; axisIndex < fontAxisCount; ++axisIndex) {
        if (fontResource->GetFontAxisAttributes(axisIndex) & DWRITE_FONT_AXIS_ATTRIBUTES_VARIABLE) {
            parameters[coordIndex].tag = SkEndian_SwapBE32(fontAxisDefaultValue[axisIndex].axisTag);
            parameters[coordIndex].min = fontAxisRange[axisIndex].minValue;
            parameters[coordIndex].def = fontAxisDefaultValue[axisIndex].value;
            parameters[coordIndex].max = fontAxisRange[axisIndex].maxValue;
            parameters[coordIndex].setHidden(fontResource->GetFontAxisAttributes(axisIndex) &
                                             DWRITE_FONT_AXIS_ATTRIBUTES_HIDDEN);
            ++coordIndex;
        }
    }

    return variableAxisCount;

#else
    return -1;
#endif
}

int DWriteFontTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    DWRITE_FONT_FACE_TYPE type = fDWriteFontFace->GetType();
    if (type != DWRITE_FONT_FACE_TYPE_CFF &&
        type != DWRITE_FONT_FACE_TYPE_TRUETYPE &&
        type != DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION)
    {
        return 0;
    }

    int ttcIndex;
    std::unique_ptr<SkStreamAsset> stream = this->openStream(&ttcIndex);
    return stream.get() ? SkFontStream::GetTableTags(stream.get(), ttcIndex, tags) : 0;
}

size_t DWriteFontTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                          size_t length, void* data) const
{
    AutoDWriteTable table(fDWriteFontFace.get(), SkEndian_SwapBE32(tag));
    if (!table.fExists) {
        return 0;
    }

    if (offset > table.fSize) {
        return 0;
    }
    size_t size = std::min(length, table.fSize - offset);
    if (data) {
        memcpy(data, table.fData + offset, size);
    }

    return size;
}

sk_sp<SkData> DWriteFontTypeface::onCopyTableData(SkFontTableTag tag) const {
    const uint8_t* data;
    UINT32 size;
    void* lock;
    BOOL exists;
    fDWriteFontFace->TryGetFontTable(SkEndian_SwapBE32(tag),
            reinterpret_cast<const void **>(&data), &size, &lock, &exists);
    if (!exists) {
        return nullptr;
    }
    struct Context {
        Context(void* lock, IDWriteFontFace* face) : fLock(lock), fFontFace(SkRefComPtr(face)) {}
        ~Context() { fFontFace->ReleaseFontTable(fLock); }
        void* fLock;
        SkTScopedComPtr<IDWriteFontFace> fFontFace;
    };
    return SkData::MakeWithProc(data, size,
                                [](const void*, void* ctx) { delete (Context*)ctx; },
                                new Context(lock, fDWriteFontFace.get()));
}

sk_sp<SkTypeface> DWriteFontTypeface::onMakeClone(const SkFontArguments& args) const {
    // Skip if the current face index does not match the ttcIndex
    if (fDWriteFontFace->GetIndex() != SkTo<UINT32>(args.getCollectionIndex())) {
        return sk_ref_sp(this);
    }

#if defined(NTDDI_WIN10_RS3) && NTDDI_VERSION >= NTDDI_WIN10_RS3

    SkTScopedComPtr<IDWriteFontFace5> fontFace5;

    if (SUCCEEDED(fDWriteFontFace->QueryInterface(&fontFace5)) && fontFace5->HasVariations()) {
        UINT32 fontAxisCount = fontFace5->GetFontAxisValueCount();
        UINT32 argsCoordCount = args.getVariationDesignPosition().coordinateCount;
        AutoSTMalloc<8, DWRITE_FONT_AXIS_VALUE> fontAxisValue(fontAxisCount);
        HRN(fontFace5->GetFontAxisValues(fontAxisValue.get(), fontAxisCount));

        for (UINT32 fontIndex = 0; fontIndex < fontAxisCount; ++fontIndex) {
            for (UINT32 argsIndex = 0; argsIndex < argsCoordCount; ++argsIndex) {
                if (SkEndian_SwapBE32(fontAxisValue[fontIndex].axisTag) ==
                    args.getVariationDesignPosition().coordinates[argsIndex].axis) {
                    fontAxisValue[fontIndex].value =
                        args.getVariationDesignPosition().coordinates[argsIndex].value;
                }
            }
        }
        SkTScopedComPtr<IDWriteFontResource> fontResource;
        HRN(fontFace5->GetFontResource(&fontResource));
        SkTScopedComPtr<IDWriteFontFace5> newFontFace5;
        HRN(fontResource->CreateFontFace(fDWriteFont->GetSimulations(),
                                         fontAxisValue.get(),
                                         fontAxisCount,
                                         &newFontFace5));

        SkTScopedComPtr<IDWriteFontFace> newFontFace;
        HRN(newFontFace5->QueryInterface(&newFontFace));
        return DWriteFontTypeface::Make(fFactory.get(),
                                        newFontFace.get(),
                                        fDWriteFont.get(),
                                        fDWriteFontFamily.get(),
                                        fLoaders,
                                        args.getPalette());
    }

#endif

    // If the palette args have changed, a new font will need to be created.
    if (args.getPalette().index != fRequestedPalette.index ||
        args.getPalette().overrideCount != fRequestedPalette.overrideCount ||
        memcmp(args.getPalette().overrides, fRequestedPalette.overrides,
               fRequestedPalette.overrideCount * sizeof(fRequestedPalette.overrides[0])))
    {
        return DWriteFontTypeface::Make(fFactory.get(),
                                        fDWriteFontFace.get(),
                                        fDWriteFont.get(),
                                        fDWriteFontFamily.get(),
                                        fLoaders,
                                        args.getPalette());
    }

    return sk_ref_sp(this);
}

std::unique_ptr<SkStreamAsset> DWriteFontTypeface::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fDWriteFontFace->GetIndex();

    UINT32 numFiles = 0;
    HRNM(fDWriteFontFace->GetFiles(&numFiles, nullptr),
         "Could not get number of font files.");
    if (numFiles != 1) {
        return nullptr;
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

    return std::unique_ptr<SkStreamAsset>(new SkDWriteFontFileStream(fontFileStream.get()));
}

std::unique_ptr<SkScalerContext> DWriteFontTypeface::onCreateScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc) const
{
    return std::make_unique<SkScalerContext_DW>(
            sk_ref_sp(const_cast<DWriteFontTypeface*>(this)), effects, desc);
}

void DWriteFontTypeface::onFilterRec(SkScalerContextRec* rec) const {
    if (rec->fFlags & SkScalerContext::kLCD_Vertical_Flag) {
        rec->fMaskFormat = SkMask::kA8_Format;
        rec->fFlags |= SkScalerContext::kGenA8FromLCD_Flag;
    }

    unsigned flagsWeDontSupport = SkScalerContext::kForceAutohinting_Flag |
                                  SkScalerContext::kEmbolden_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;
    rec->fFlags &= ~flagsWeDontSupport;

    SkFontHinting h = rec->getHinting();
    // DirectWrite2 allows for hinting to be turned off. Force everything else to normal.
    if (h != SkFontHinting::kNone || !fFactory2 || !fDWriteFontFace2) {
        h = SkFontHinting::kNormal;
    }
    rec->setHinting(h);

#if defined(SK_FONT_HOST_USE_SYSTEM_SETTINGS)
    IDWriteFactory* factory = sk_get_dwrite_factory();
    if (factory != nullptr) {
        SkTScopedComPtr<IDWriteRenderingParams> defaultRenderingParams;
        if (SUCCEEDED(factory->CreateRenderingParams(&defaultRenderingParams))) {
            float gamma = defaultRenderingParams->GetGamma();
            rec->setDeviceGamma(gamma);
            rec->setPaintGamma(gamma);

            rec->setContrast(defaultRenderingParams->GetEnhancedContrast());
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
//PDF Support

static void glyph_to_unicode_map(IDWriteFontFace* fontFace, DWRITE_UNICODE_RANGE range,
                                 UINT32* remainingGlyphCount, UINT32 numGlyphs,
                                 SkUnichar* glyphToUnicode)
{
    constexpr const int batchSize = 128;
    UINT32 codepoints[batchSize];
    UINT16 glyphs[batchSize];
    for (UINT32 c = range.first; c <= range.last && *remainingGlyphCount != 0; c += batchSize) {
        UINT32 numBatchedCodePoints = std::min<UINT32>(range.last - c + 1, batchSize);
        for (UINT32 i = 0; i < numBatchedCodePoints; ++i) {
            codepoints[i] = c + i;
        }
        HRVM(fontFace->GetGlyphIndices(codepoints, numBatchedCodePoints, glyphs),
             "Failed to get glyph indexes.");
        for (UINT32 i = 0; i < numBatchedCodePoints; ++i) {
            UINT16 glyph = glyphs[i];
            // Intermittent DW bug on Windows 10. See crbug.com/470146.
            if (glyph >= numGlyphs) {
                return;
            }
            if (0 < glyph && glyphToUnicode[glyph] == 0) {
                glyphToUnicode[glyph] = c + i;  // Always use lowest-index unichar.
                --*remainingGlyphCount;
            }
        }
    }
}

void DWriteFontTypeface::getGlyphToUnicodeMap(SkUnichar* glyphToUnicode) const {
    IDWriteFontFace* face = fDWriteFontFace.get();
    UINT32 numGlyphs = face->GetGlyphCount();
    sk_bzero(glyphToUnicode, sizeof(SkUnichar) * numGlyphs);
    UINT32 remainingGlyphCount = numGlyphs;

    if (fDWriteFontFace1) {
        IDWriteFontFace1* face1 = fDWriteFontFace1.get();
        UINT32 numRanges = 0;
        HRESULT hr = face1->GetUnicodeRanges(0, nullptr, &numRanges);
        if (hr != E_NOT_SUFFICIENT_BUFFER && FAILED(hr)) {
            HRVM(hr, "Failed to get number of ranges.");
        }
        std::unique_ptr<DWRITE_UNICODE_RANGE[]> ranges(new DWRITE_UNICODE_RANGE[numRanges]);
        HRVM(face1->GetUnicodeRanges(numRanges, ranges.get(), &numRanges), "Failed to get ranges.");
        for (UINT32 i = 0; i < numRanges; ++i) {
            glyph_to_unicode_map(face1, ranges[i], &remainingGlyphCount, numGlyphs, glyphToUnicode);
        }
    } else {
        glyph_to_unicode_map(face, {0, 0x10FFFF}, &remainingGlyphCount, numGlyphs, glyphToUnicode);
    }
}

std::unique_ptr<SkAdvancedTypefaceMetrics> DWriteFontTypeface::onGetAdvancedMetrics() const {

    std::unique_ptr<SkAdvancedTypefaceMetrics> info(nullptr);

    DWRITE_FONT_METRICS dwfm;
    fDWriteFontFace->GetMetrics(&dwfm);

    info.reset(new SkAdvancedTypefaceMetrics);

    info->fAscent = SkToS16(dwfm.ascent);
    info->fDescent = SkToS16(dwfm.descent);
    info->fCapHeight = SkToS16(dwfm.capHeight);

    {
        SkTScopedComPtr<IDWriteLocalizedStrings> postScriptNames;
        BOOL exists = FALSE;
        if (FAILED(fDWriteFont->GetInformationalStrings(
                        DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME,
                        &postScriptNames,
                        &exists)) ||
            !exists ||
            FAILED(sk_get_locale_string(postScriptNames.get(), nullptr, &info->fPostScriptName)))
        {
            SkDEBUGF("Unable to get postscript name for typeface %p\n", this);
        }
    }

    // SkAdvancedTypefaceMetrics::fFontName must actually be a family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    if (FAILED(fDWriteFontFamily->GetFamilyNames(&familyNames)) ||
        FAILED(sk_get_locale_string(familyNames.get(), nullptr, &info->fFontName)))
    {
        SkDEBUGF("Unable to get family name for typeface 0x%p\n", this);
    }
    if (info->fPostScriptName.isEmpty()) {
        info->fPostScriptName = info->fFontName;
    }

    DWRITE_FONT_FACE_TYPE fontType = fDWriteFontFace->GetType();
    if (fontType != DWRITE_FONT_FACE_TYPE_TRUETYPE &&
        fontType != DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION)
    {
        return info;
    }

    // Simulated fonts aren't really TrueType fonts.
    if (fDWriteFontFace->GetSimulations() == DWRITE_FONT_SIMULATIONS_NONE) {
        info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
    }

    AutoTDWriteTable<SkOTTableHead> headTable(fDWriteFontFace.get());
    AutoTDWriteTable<SkOTTablePostScript> postTable(fDWriteFontFace.get());
    AutoTDWriteTable<SkOTTableHorizontalHeader> hheaTable(fDWriteFontFace.get());
    AutoTDWriteTable<SkOTTableOS2> os2Table(fDWriteFontFace.get());
    if (!headTable.fExists || !postTable.fExists || !hheaTable.fExists || !os2Table.fExists) {
        return info;
    }

    SkOTUtils::SetAdvancedTypefaceFlags(os2Table->version.v4.fsType, info.get());

    // There are versions of DirectWrite which support named instances for system variation fonts,
    // but no means to indicate that such a typeface is a variation.
    AutoTDWriteTable<SkOTTableFontVariations> fvarTable(fDWriteFontFace.get());
    if (fvarTable.fExists) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kVariable_FontFlag;
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
    //Serif
    using SerifStyle = SkPanose::Data::TextAndDisplay::SerifStyle;
    SerifStyle serifStyle = os2Table->version.v0.panose.data.textAndDisplay.bSerifStyle;
    if (SkPanose::FamilyType::TextAndDisplay == os2Table->version.v0.panose.bFamilyType) {
        if (SerifStyle::Cove == serifStyle ||
            SerifStyle::ObtuseCove == serifStyle ||
            SerifStyle::SquareCove == serifStyle ||
            SerifStyle::ObtuseSquareCove == serifStyle ||
            SerifStyle::Square == serifStyle ||
            SerifStyle::Thin == serifStyle ||
            SerifStyle::Bone == serifStyle ||
            SerifStyle::Exaggerated == serifStyle ||
            SerifStyle::Triangle == serifStyle)
        {
            info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
        }
    //Script
    } else if (SkPanose::FamilyType::Script == os2Table->version.v0.panose.bFamilyType) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    }

    info->fItalicAngle = SkEndian_SwapBE32(postTable->italicAngle) >> 16;

    info->fBBox = SkIRect::MakeLTRB((int32_t)SkEndian_SwapBE16((uint16_t)headTable->xMin),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->yMax),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->xMax),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->yMin));
    return info;
}

class StreamFontFileLoader : public IDWriteFontFileLoader {
public:
    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override;
    SK_STDMETHODIMP_(ULONG) AddRef() override;
    SK_STDMETHODIMP_(ULONG) Release() override;

    // IDWriteFontFileLoader methods
    SK_STDMETHODIMP CreateStreamFromKey(
        void const* fontFileReferenceKey,
        UINT32 fontFileReferenceKeySize,
        IDWriteFontFileStream** fontFileStream) override;

    // Takes ownership of stream.
    static HRESULT Create(std::unique_ptr<SkStreamAsset> stream,
        StreamFontFileLoader** streamFontFileLoader) {
        *streamFontFileLoader = new StreamFontFileLoader(std::move(stream));
        if (nullptr == *streamFontFileLoader) {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

private:
    StreamFontFileLoader(std::unique_ptr<SkStreamAsset> stream)
        : fStream(std::move(stream)), fRefCount(1)
    {}
    virtual ~StreamFontFileLoader() { }

    std::unique_ptr<SkStreamAsset> fStream;
    ULONG fRefCount;
};

SK_STDMETHODIMP StreamFontFileLoader::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileLoader)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

SK_STDMETHODIMP_(ULONG) StreamFontFileLoader::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

SK_STDMETHODIMP_(ULONG) StreamFontFileLoader::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

SK_STDMETHODIMP StreamFontFileLoader::CreateStreamFromKey(
    void const* fontFileReferenceKey,
    UINT32 fontFileReferenceKeySize,
    IDWriteFontFileStream** fontFileStream)
{
    SkTScopedComPtr<SkDWriteFontFileStreamWrapper> stream;
    HR(SkDWriteFontFileStreamWrapper::Create(fStream->duplicate().release(), &stream));
    *fontFileStream = stream.release();
    return S_OK;
}

class StreamFontFileEnumerator : public IDWriteFontFileEnumerator {
public:
    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override;
    SK_STDMETHODIMP_(ULONG) AddRef() override;
    SK_STDMETHODIMP_(ULONG) Release() override;

    // IDWriteFontFileEnumerator methods
    SK_STDMETHODIMP MoveNext(BOOL* hasCurrentFile) override;
    SK_STDMETHODIMP GetCurrentFontFile(IDWriteFontFile** fontFile) override;

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

SK_STDMETHODIMP StreamFontFileEnumerator::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileEnumerator)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

SK_STDMETHODIMP_(ULONG) StreamFontFileEnumerator::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

SK_STDMETHODIMP_(ULONG) StreamFontFileEnumerator::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

SK_STDMETHODIMP StreamFontFileEnumerator::MoveNext(BOOL* hasCurrentFile) {
    *hasCurrentFile = FALSE;

    if (!fHasNext) {
        return S_OK;
    }
    fHasNext = false;

    UINT32 fontFileReferenceKey = 0;
    HR(fFactory->CreateCustomFontFileReference(
        &fontFileReferenceKey, //cannot be nullptr
        sizeof(fontFileReferenceKey), //even if this is 0
        fFontFileLoader.get(),
        &fCurrentFile));

    *hasCurrentFile = TRUE;
    return S_OK;
}

SK_STDMETHODIMP StreamFontFileEnumerator::GetCurrentFontFile(IDWriteFontFile** fontFile) {
    if (fCurrentFile.get() == nullptr) {
        *fontFile = nullptr;
        return E_FAIL;
    }

    *fontFile = SkRefComPtr(fCurrentFile.get());
    return  S_OK;
}

class StreamFontCollectionLoader : public IDWriteFontCollectionLoader {
public:
    // IUnknown methods
    SK_STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override;
    SK_STDMETHODIMP_(ULONG) AddRef() override;
    SK_STDMETHODIMP_(ULONG) Release() override;

    // IDWriteFontCollectionLoader methods
    SK_STDMETHODIMP CreateEnumeratorFromKey(
        IDWriteFactory* factory,
        void const* collectionKey,
        UINT32 collectionKeySize,
        IDWriteFontFileEnumerator** fontFileEnumerator) override;

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

SK_STDMETHODIMP StreamFontCollectionLoader::QueryInterface(REFIID iid, void** ppvObject) {
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontCollectionLoader)) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}

SK_STDMETHODIMP_(ULONG) StreamFontCollectionLoader::AddRef() {
    return InterlockedIncrement(&fRefCount);
}

SK_STDMETHODIMP_(ULONG) StreamFontCollectionLoader::Release() {
    ULONG newCount = InterlockedDecrement(&fRefCount);
    if (0 == newCount) {
        delete this;
    }
    return newCount;
}

template <typename T> class SkAutoIDWriteUnregister {
public:
    SkAutoIDWriteUnregister(IDWriteFactory* factory, T* unregister)
        : fFactory(factory), fUnregister(unregister)
    { }
    SkAutoIDWriteUnregister(const SkAutoIDWriteUnregister&) = delete;
    SkAutoIDWriteUnregister& operator=(const SkAutoIDWriteUnregister&) = delete;
    SkAutoIDWriteUnregister(SkAutoIDWriteUnregister&&) = delete;
    SkAutoIDWriteUnregister& operator=(SkAutoIDWriteUnregister&&) = delete;

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

SK_STDMETHODIMP StreamFontCollectionLoader::CreateEnumeratorFromKey(
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

static HRESULT apply_fontargument_variation(SkTScopedComPtr<IDWriteFontFace>& fontFace,
    const SkFontArguments& args)
{
#if defined(NTDDI_WIN10_RS3) && NTDDI_VERSION >= NTDDI_WIN10_RS3

    SkTScopedComPtr<IDWriteFontFace5> fontFace5;
    if (FAILED(fontFace->QueryInterface(&fontFace5)) || !fontFace5->HasVariations()) {
        return S_OK;
    }

    UINT32 fontAxisCount = fontFace5->GetFontAxisValueCount();
    UINT32 argsCoordCount = args.getVariationDesignPosition().coordinateCount;
    AutoSTMalloc<8, DWRITE_FONT_AXIS_VALUE> variation(fontAxisCount);
    SkTScopedComPtr<IDWriteFontResource> fontResource;
    HR(fontFace5->GetFontResource(&fontResource));
    HR(fontResource->GetDefaultFontAxisValues(variation, fontAxisCount));

    for (UINT32 fontAxisIndex = 0; fontAxisIndex < fontAxisCount; ++fontAxisIndex) {
        DWRITE_FONT_AXIS_VALUE& fontCoordinate = variation[fontAxisIndex];

        for (UINT32 argsCoordIndex = argsCoordCount; argsCoordIndex --> 0;) {
            const SkFontArguments::VariationPosition::Coordinate& argsCoordinate =
                args.getVariationDesignPosition().coordinates[argsCoordIndex];
            if (SkEndian_SwapBE32(fontCoordinate.axisTag) == argsCoordinate.axis) {
                fontCoordinate.value = argsCoordinate.value;
                break;
            }
        }
    }

    SkTScopedComPtr<IDWriteFontFace5> fontFace5_Out;
    HR(fontResource->CreateFontFace(DWRITE_FONT_SIMULATIONS_NONE,
        variation.get(), fontAxisCount,
        &fontFace5_Out));
    fontFace.reset();
    HR(fontFace5_Out->QueryInterface(&fontFace));
#endif
    return S_OK;
}

sk_sp<SkTypeface> DWriteFontTypeface::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                     const SkFontArguments& args) {
    // TODO: do we need to use some user provided factory?
    IDWriteFactory* factory = sk_get_dwrite_factory();
    if (nullptr == factory) {
        return nullptr;
    }

    SkTScopedComPtr<StreamFontFileLoader> fontFileLoader;
    HRN(StreamFontFileLoader::Create(std::move(stream), &fontFileLoader));
    HRN(factory->RegisterFontFileLoader(fontFileLoader.get()));
    SkAutoIDWriteUnregister<StreamFontFileLoader> autoUnregisterFontFileLoader(
        factory, fontFileLoader.get());

    SkTScopedComPtr<StreamFontCollectionLoader> fontCollectionLoader;
    HRN(StreamFontCollectionLoader::Create(fontFileLoader.get(), &fontCollectionLoader));
    HRN(factory->RegisterFontCollectionLoader(fontCollectionLoader.get()));
    SkAutoIDWriteUnregister<StreamFontCollectionLoader> autoUnregisterFontCollectionLoader(
        factory, fontCollectionLoader.get());

    SkTScopedComPtr<IDWriteFontCollection> fontCollection;
    HRN(factory->CreateCustomFontCollection(fontCollectionLoader.get(), nullptr, 0,
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

            apply_fontargument_variation(fontFace, args);

            return DWriteFontTypeface::Make(
                factory, fontFace.get(), font.get(), fontFamily.get(),
                sk_make_sp<DWriteFontTypeface::Loaders>(
                    factory,
                    autoUnregisterFontFileLoader.detatch(),
                    autoUnregisterFontCollectionLoader.detatch()),
                args.getPalette());
        }
    }

    return nullptr;
}

#endif//defined(SK_BUILD_FOR_WIN)
