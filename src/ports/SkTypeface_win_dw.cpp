/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

// SkTypes will include Windows.h, which will pull in all of the GDI defines.
// GDI #defines GetGlyphIndices to GetGlyphIndicesA or GetGlyphIndicesW, but
// IDWriteFontFace has a method called GetGlyphIndices. Since this file does
// not use GDI, undefing GetGlyphIndices makes things less confusing.
#undef GetGlyphIndices

#include "SkDWrite.h"
#include "SkDWriteFontFileStream.h"
#include "SkFontDescriptor.h"
#include "SkFontStream.h"
#include "SkOTTable_fvar.h"
#include "SkOTTable_head.h"
#include "SkOTTable_hhea.h"
#include "SkOTTable_OS_2.h"
#include "SkOTTable_post.h"
#include "SkOTUtils.h"
#include "SkScalerContext.h"
#include "SkScalerContext_win_dw.h"
#include "SkTypeface_win_dw.h"
#include "SkUtils.h"

void DWriteFontTypeface::onGetFamilyName(SkString* familyName) const {
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRV(fDWriteFontFamily->GetFamilyNames(&familyNames));

    sk_get_locale_string(familyNames.get(), nullptr/*fMgr->fLocaleName.get()*/, familyName);
}

void DWriteFontTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    // Get the family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRV(fDWriteFontFamily->GetFamilyNames(&familyNames));

    SkString utf8FamilyName;
    sk_get_locale_string(familyNames.get(), nullptr/*fMgr->fLocaleName.get()*/, &utf8FamilyName);

    desc->setFamilyName(utf8FamilyName.c_str());
    desc->setStyle(this->fontStyle());
    *isLocalStream = SkToBool(fDWriteFontFileLoader.get());
}

static SkUnichar next_utf8(const void** chars) {
    return SkUTF8_NextUnichar((const char**)chars);
}

static SkUnichar next_utf16(const void** chars) {
    return SkUTF16_NextUnichar((const uint16_t**)chars);
}

static SkUnichar next_utf32(const void** chars) {
    const SkUnichar** uniChars = (const SkUnichar**)chars;
    SkUnichar uni = **uniChars;
    *uniChars += 1;
    return uni;
}

typedef SkUnichar (*EncodingProc)(const void**);

static EncodingProc find_encoding_proc(SkTypeface::Encoding enc) {
    static const EncodingProc gProcs[] = {
        next_utf8, next_utf16, next_utf32
    };
    SkASSERT((size_t)enc < SK_ARRAY_COUNT(gProcs));
    return gProcs[enc];
}

int DWriteFontTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                        uint16_t glyphs[], int glyphCount) const
{
    if (nullptr == glyphs) {
        EncodingProc next_ucs4_proc = find_encoding_proc(encoding);
        for (int i = 0; i < glyphCount; ++i) {
            const SkUnichar c = next_ucs4_proc(&chars);
            BOOL exists;
            fDWriteFont->HasCharacter(c, &exists);
            if (!exists) {
                return i;
            }
        }
        return glyphCount;
    }

    switch (encoding) {
    case SkTypeface::kUTF8_Encoding:
    case SkTypeface::kUTF16_Encoding: {
        static const int scratchCount = 256;
        UINT32 scratch[scratchCount];
        EncodingProc next_ucs4_proc = find_encoding_proc(encoding);
        for (int baseGlyph = 0; baseGlyph < glyphCount; baseGlyph += scratchCount) {
            int glyphsLeft = glyphCount - baseGlyph;
            int limit = SkTMin(glyphsLeft, scratchCount);
            for (int i = 0; i < limit; ++i) {
                scratch[i] = next_ucs4_proc(&chars);
            }
            fDWriteFontFace->GetGlyphIndices(scratch, limit, &glyphs[baseGlyph]);
        }
        break;
    }
    case SkTypeface::kUTF32_Encoding: {
        const UINT32* utf32 = reinterpret_cast<const UINT32*>(chars);
        fDWriteFontFace->GetGlyphIndices(utf32, glyphCount, glyphs);
        break;
    }
    default:
        SK_ABORT("Invalid Text Encoding");
    }

    for (int i = 0; i < glyphCount; ++i) {
        if (0 == glyphs[i]) {
            return i;
        }
    }
    return glyphCount;
}

int DWriteFontTypeface::onCountGlyphs() const {
    return fDWriteFontFace->GetGlyphCount();
}

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

        SkSMallocWCHAR wString(stringLen+1);
        HRBM(fStrings->GetString(fIndex, wString.get(), stringLen+1), "Could not get string.");

        HRB(sk_wchar_to_skstring(wString.get(), stringLen, &localizedString->fString));

        // Locale
        UINT32 localeLen;
        HRBM(fStrings->GetLocaleNameLength(fIndex, &localeLen), "Could not get locale length.");

        SkSMallocWCHAR wLocale(localeLen+1);
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
    SkTypeface::LocalizedStrings* nameIter =
        SkOTUtils::LocalizedStrings_NameTable::CreateForFamilyNames(*this);
    if (nullptr == nameIter) {
        SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
        HRNM(fDWriteFontFamily->GetFamilyNames(&familyNames), "Could not obtain family names.");
        nameIter = new LocalizedStrings_IDWriteLocalizedStrings(familyNames.release());
    }
    return nameIter;
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
    std::unique_ptr<SkStream> stream(this->openStream(&ttcIndex));
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
    size_t size = SkTMin(length, table.fSize - offset);
    if (data) {
        memcpy(data, table.fData + offset, size);
    }

    return size;
}

SkStreamAsset* DWriteFontTypeface::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fDWriteFontFace->GetIndex();

    UINT32 numFiles;
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

    return new SkDWriteFontFileStream(fontFileStream.get());
}

SkScalerContext* DWriteFontTypeface::onCreateScalerContext(const SkScalerContextEffects& effects,
                                                           const SkDescriptor* desc) const {
    return new SkScalerContext_DW(sk_ref_sp(const_cast<DWriteFontTypeface*>(this)), effects, desc);
}

void DWriteFontTypeface::onFilterRec(SkScalerContext::Rec* rec) const {
    if (rec->fFlags & SkScalerContext::kLCD_Vertical_Flag) {
        rec->fMaskFormat = SkMask::kA8_Format;
        rec->fFlags |= SkScalerContext::kGenA8FromLCD_Flag;
    }

    unsigned flagsWeDontSupport = SkScalerContext::kVertical_Flag |
                                  SkScalerContext::kDevKernText_Flag |
                                  SkScalerContext::kForceAutohinting_Flag |
                                  SkScalerContext::kEmbolden_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;
    rec->fFlags &= ~flagsWeDontSupport;

    SkPaint::Hinting h = rec->getHinting();
    // DirectWrite2 allows for hinting to be turned off. Force everything else to normal.
    if (h != SkPaint::kNo_Hinting || !fFactory2 || !fDWriteFontFace2) {
        h = SkPaint::kNormal_Hinting;
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

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
// TODO(bungeman): This never does what anyone wants.
// What is really wanted is the text to glyphs mapping
static void populate_glyph_to_unicode(IDWriteFontFace* fontFace,
                                      const unsigned glyphCount,
                                      SkTDArray<SkUnichar>* glyphToUnicode) {
    //Do this like free type instead
    SkAutoTMalloc<SkUnichar> glyphToUni(
            (SkUnichar*)sk_calloc_throw(sizeof(SkUnichar) * glyphCount));
    int maxGlyph = -1;
    unsigned remainingGlyphCount = glyphCount;
    for (UINT32 c = 0; c < 0x10FFFF && remainingGlyphCount != 0; ++c) {
        UINT16 glyph = 0;
        HRVM(fontFace->GetGlyphIndices(&c, 1, &glyph),
             "Failed to get glyph index.");
        // Intermittent DW bug on Windows 10. See crbug.com/470146.
        if (glyph >= glyphCount) {
          return;
        }
        if (0 < glyph && glyphToUni[glyph] == 0) {
            maxGlyph = SkTMax(static_cast<int>(glyph), maxGlyph);
            glyphToUni[glyph] = c;  // Always use lowest-index unichar.
            --remainingGlyphCount;
        }
    }
    SkTDArray<SkUnichar>(glyphToUni, maxGlyph + 1).swap(*glyphToUnicode);
}

std::unique_ptr<SkAdvancedTypefaceMetrics> DWriteFontTypeface::onGetAdvancedMetrics() const {

    std::unique_ptr<SkAdvancedTypefaceMetrics> info(nullptr);

    HRESULT hr = S_OK;

    const unsigned glyphCount = fDWriteFontFace->GetGlyphCount();

    DWRITE_FONT_METRICS dwfm;
    fDWriteFontFace->GetMetrics(&dwfm);

    info.reset(new SkAdvancedTypefaceMetrics);

    info->fAscent = SkToS16(dwfm.ascent);
    info->fDescent = SkToS16(dwfm.descent);
    info->fCapHeight = SkToS16(dwfm.capHeight);

    // SkAdvancedTypefaceMetrics::fFontName is in theory supposed to be
    // the PostScript name of the font. However, due to the way it is currently
    // used, it must actually be a family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    hr = fDWriteFontFamily->GetFamilyNames(&familyNames);

    UINT32 familyNameLen;
    hr = familyNames->GetStringLength(0, &familyNameLen);

    SkSMallocWCHAR familyName(familyNameLen+1);
    hr = familyNames->GetString(0, familyName.get(), familyNameLen+1);

    hr = sk_wchar_to_skstring(familyName.get(), familyNameLen, &info->fFontName);

    populate_glyph_to_unicode(fDWriteFontFace.get(), glyphCount, &(info->fGlyphToUnicode));

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

    // There are versions of DirectWrite which support named instances for system variation fonts,
    // but no means to indicate that such a typeface is a variation.
    AutoTDWriteTable<SkOTTableFontVariations> fvarTable(fDWriteFontFace.get());
    if (fvarTable.fExists) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag;
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
#endif//defined(SK_BUILD_FOR_WIN32)
